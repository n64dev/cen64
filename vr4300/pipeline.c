//
// vr4300/pipeline.c: VR4300 processor pipeline.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/controller.h"
#include "vr4300/cp0.h"
#include "vr4300/cpu.h"
#include "vr4300/decoder.h"
#include "vr4300/fault.h"
#include "vr4300/opcodes.h"
#include "vr4300/pipeline.h"
#include "vr4300/segment.h"

static void vr4300_cycle_slow_wb(struct vr4300 *vr4300);
static void vr4300_cycle_slow_dc(struct vr4300 *vr4300);
static void vr4300_cycle_slow_ex(struct vr4300 *vr4300);
static void vr4300_cycle_slow_rf(struct vr4300 *vr4300);
static void vr4300_cycle_slow_ic(struct vr4300 *vr4300);

static void vr4300_cycle_busywait(struct vr4300 *vr4300);
static void vr4300_cycle_slow_ex_fixdc(struct vr4300 *vr4300);

// Prints out instructions and their virtual address as they are executed.
// Note: These instructions should _may_ be speculative and killed later...
//#define PRINT_EXEC

// Instruction cache stage.
static inline int vr4300_ic_stage(struct vr4300 *vr4300) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;

  const struct segment *segment = icrf_latch->segment;
  struct vr4300_opcode *opcode = &rfex_latch->opcode;
  uint64_t pc = icrf_latch->pc;
  uint32_t decode_iw;

  // Finish decoding instruction in RF.
  decode_iw = rfex_latch->iw &= rfex_latch->iw_mask;
  *opcode = *vr4300_decode_instruction(decode_iw);
  rfex_latch->iw_mask = ~0U;

  // Latch common pipeline values.
  icrf_latch->common.fault = VR4300_FAULT_NONE;
  icrf_latch->common.pc = pc;

  // If decoding of prior instruction indicates this is a BD slot...
  icrf_latch->common.cause_data = (opcode->flags & OPCODE_INFO_BRANCH)
    ? 0x80000000
    : 0x00000000;

  // Look up the segment that we're in.
  if ((pc - segment->start) > segment->length) {
    uint32_t cp0_status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

    if (unlikely((segment = get_segment(pc, cp0_status)) == NULL)) {
      VR4300_IADE(vr4300);
      return 1;
    }

    icrf_latch->segment = segment;
  }

  icrf_latch->pc += 4;
  return 0;
}

// Register fetch and decode stage.
static inline int vr4300_rf_stage(struct vr4300 *vr4300) {
  const struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;

  const struct segment *segment = icrf_latch->segment;
  const struct vr4300_icache_line *line;
  uint32_t paddr;

  rfex_latch->common = icrf_latch->common;

  if (!segment->cached) {
    VR4300_UNC(vr4300);
    return 1;
  }

  // TODO: Implement the TLB.
  assert(segment->mapped == 0);

  // Probe the instruction cache for the data.
  paddr = icrf_latch->common.pc - segment->offset;

  if ((line = vr4300_icache_probe(&vr4300->icache,
    icrf_latch->common.pc, paddr)) == NULL) {
    VR4300_ICB(vr4300);
    return 1;
  }

  memcpy(&rfex_latch->iw, line->data + (paddr & 0x1C), sizeof(rfex_latch->iw));
  return 0;
}

// Execution stage.
static inline int vr4300_ex_stage(struct vr4300 *vr4300) {
  const struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  const struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  // Used to select either rs/fs, rt/ft.
  cen64_align(static const unsigned rs_select_lut[4], 16) = {
    0, VR4300_REGISTER_CP1_0, // Source indexes
    21, 11                    // Shift amounts
  };

  cen64_align(static const unsigned rt_select_lut[4], 16) = {
    0, 0,                     // Padding (unused)
    VR4300_REGISTER_CP1_0, 0, // Source indexes
  };

  unsigned rs, rt, rslutidx, rtlutidx, fr;
  uint64_t rs_reg, rt_reg, temp;
  uint32_t flags, iw;

  exdc_latch->common = rfex_latch->common;
  fr = (status >> 26 & 0x1) ^ 1;
  iw = rfex_latch->iw;

  flags = rfex_latch->opcode.flags;
  if (exdc_latch->request.type != VR4300_BUS_REQUEST_READ)
    flags &= ~(OPCODE_INFO_NEEDRS | OPCODE_INFO_NEEDRT);

  // CP1 register, or no?
  rslutidx = flags & 0x1;
  rtlutidx = flags & 0x2;

  rs = (iw >> rs_select_lut[2 + rslutidx] & 0x1F) + rs_select_lut[rslutidx];
  rt = (iw >> 16 & 0x1F) + rt_select_lut[rtlutidx];

  // If FR bit is set, we depend on even registers only.
  rt &= ~((rtlutidx >> 1) & fr);
  rs &= ~(rslutidx & fr);

  // Check to see if we should hold off execution due to a LDI.
  if (((dcwb_latch->dest == rs) && (flags & OPCODE_INFO_NEEDRS)) ||
    ((dcwb_latch->dest == rt) && (flags & OPCODE_INFO_NEEDRT))) {
    VR4300_LDI(vr4300);
    return 1;
  }

  // No LDI, so we just need to forward results from DC/WB.
  // This is done to preserve RF state and fwd without branching.
  if (!dcwb_latch->common.fault) {
    temp = vr4300->regs[dcwb_latch->dest];
    vr4300->regs[dcwb_latch->dest] = dcwb_latch->result;
    vr4300->regs[VR4300_REGISTER_R0] = 0x0000000000000000ULL;

    rs_reg = vr4300->regs[rs];
    rt_reg = vr4300->regs[rt];

    vr4300->regs[dcwb_latch->dest] = temp;
  }

  else {
    rs_reg = vr4300->regs[rs];
    rt_reg = vr4300->regs[rt];
  }

  // Finally, execute the instruction.
#ifdef PRINT_EXEC
  debug("%.16llX: %s\n", (unsigned long long) rfex_latch->common.pc,
    vr4300_opcode_mnemonics[rfex_latch->opcode.id]);
#endif

  exdc_latch->dest = VR4300_REGISTER_R0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_NONE;
  return vr4300_function_table[rfex_latch->opcode.id](vr4300, rs_reg, rt_reg);
}

// Data cache fetch stage.
static inline int vr4300_dc_stage(struct vr4300 *vr4300) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;

  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint32_t cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];
  const struct segment *segment = exdc_latch->segment;
  uint64_t address = exdc_latch->request.address;

  dcwb_latch->common = exdc_latch->common;
  dcwb_latch->result = exdc_latch->result;
  dcwb_latch->dest = exdc_latch->dest;

  // The reset exception has a very high priority and will abort basically
  // anything that's active, even if we have an interlock or something that's
  // current active. Thus, we check for it here and handle it early.
  if (unlikely(vr4300->signals & VR4300_SIGNAL_COLDRESET)) {
    VR4300_RST(vr4300);
    return 1;
  }

  // Check if we should raise an interrupt (and effectively kill this insn).
  if (unlikely(cause & status & 0xFF00) && (status & 0x1) && !(status & 0x6)) {
    VR4300_INTR(vr4300);
    return 1;
  }

  // Look up the segment that we're in.
  if (exdc_latch->request.type != VR4300_BUS_REQUEST_NONE) {
    if ((address - segment->start) > segment->length) {
      uint32_t cp0_status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

      if (unlikely((segment = get_segment(address, cp0_status)) == NULL)) {
        VR4300_DADE(vr4300);
        return 1;
      }

      exdc_latch->segment = segment;
    }

    if (exdc_latch->request.type == VR4300_BUS_REQUEST_READ) {
      struct vr4300_bus_request *request = &exdc_latch->request;
      const struct vr4300_dcache_line *line;

      uint64_t vaddr = request->address;
      uint32_t paddr;

      // TODO: Implement the TLB.
      assert(segment->mapped == 0);

      paddr = vaddr - segment->offset;

      // Index the data cache; on a miss, stall until the data is available.
      if ((line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr)) == NULL) {
        request->address -= segment->offset;
        VR4300_DCB(vr4300);
        return 1;
      }

      assert(0 && "Implement data cache reads.");
      return 0;
    }

    else {
      assert(exdc_latch->request.type == VR4300_BUS_REQUEST_WRITE &&
        "Unsupported DC stage request type.");

      exdc_latch->request.address -= segment->offset;

      // TODO/FIXME: Not accurate.
      if (exdc_latch->request.size > 4) {
        bus_write_word(vr4300->bus, exdc_latch->request.address,
          exdc_latch->request.data >> 32, exdc_latch->request.dqm);

        exdc_latch->request.dqm >>= 32;
        exdc_latch->request.address += 4;
      }

      bus_write_word(vr4300->bus, exdc_latch->request.address,
        exdc_latch->request.data, exdc_latch->request.dqm);
    }
  }

  return 0;
}

// Writeback stage.
static inline int vr4300_wb_stage(struct vr4300 *vr4300) {
  const struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;

  vr4300->regs[dcwb_latch->dest] = dcwb_latch->result;
  vr4300->regs[VR4300_REGISTER_R0] = 0x0000000000000000ULL;
  return 0;
}

// Advances the processor pipeline by one pclock.
// May have exceptions, so check for aborted stages.
void vr4300_cycle_slow_wb(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_dcwb_latch *dcwb_latch = &pipeline->dcwb_latch;

  // If we haven't had exceptions for at least a
  // full pipeline's length, switch back to fast mode.
  if (pipeline->exception_history++ > 4)
    pipeline->fault_present = false;

  if (dcwb_latch->common.fault == VR4300_FAULT_NONE) {
    if (vr4300_wb_stage(vr4300))
      return;
  }

  vr4300_cycle_slow_dc(vr4300);
}

// Advances the processor pipeline by one pclock.
// May have exceptions, so check for aborted stages.
//
// Starts from DC stage (WB resolved an interlock).
void vr4300_cycle_slow_dc(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_dcwb_latch *dcwb_latch = &pipeline->dcwb_latch;
  struct vr4300_exdc_latch *exdc_latch = &pipeline->exdc_latch;

  if (exdc_latch->common.fault == VR4300_FAULT_NONE) {
    if (vr4300_dc_stage(vr4300))
      return;
  }

  else
    dcwb_latch->common = exdc_latch->common;

  vr4300_cycle_slow_ex(vr4300);
}

// Advances the processor pipeline by one pclock.
// May have exceptions, so check for aborted stages.
//
// Starts from EX stage (DC resolved an interlock).
void vr4300_cycle_slow_ex(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_exdc_latch *exdc_latch = &pipeline->exdc_latch;
  struct vr4300_rfex_latch *rfex_latch = &pipeline->rfex_latch;

  if (rfex_latch->common.fault == VR4300_FAULT_NONE) {
    if (vr4300_ex_stage(vr4300))
      return;
  }

  else
    exdc_latch->common = rfex_latch->common;

  vr4300_cycle_slow_rf(vr4300);
}

// Advances the processor pipeline by one pclock.
// May have exceptions, so check for aborted stages.
//
// Starts from RF stage (EX resolved an interlock).
void vr4300_cycle_slow_rf(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_rfex_latch *rfex_latch = &pipeline->rfex_latch;
  struct vr4300_icrf_latch *icrf_latch = &pipeline->icrf_latch;

  if (icrf_latch->common.fault == VR4300_FAULT_NONE) {
    if (vr4300_rf_stage(vr4300))
      return;
  }

  else
    rfex_latch->common = icrf_latch->common;

  vr4300_cycle_slow_ic(vr4300);
}

// Advances the processor pipeline by one pclock.
// May have exceptions, so check for aborted stages.
//
// Starts from IC stage (RF resolved an interlock).
void vr4300_cycle_slow_ic(struct vr4300 *vr4300) {
  if (vr4300_ic_stage(vr4300))
    return;

  vr4300->regs[PIPELINE_CYCLE_TYPE] = 0;
}

// Special-cased busy wait handler.
void vr4300_cycle_busywait(struct vr4300 *vr4300) {
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint32_t cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];

  // Check if the busy wait period is over (due to an interrupt condition).
  if (unlikely(cause & status & 0xFF00) && (status & 0x1) && !(status & 0x6)) {
    //debug("Busy wait done @ %llu cycles\n", vr4300->cycles);

    VR4300_INTR(vr4300);
  }
}

// Advances the processor pipeline by one pclock.
// May have exceptions, so check for aborted stages.
//
// Starts from EX stage (DC resolved an interlock).
// Fixes up the DC/WB latches after memory reads.
void vr4300_cycle_slow_ex_fixdc(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_exdc_latch *exdc_latch = &pipeline->exdc_latch;
  struct vr4300_dcwb_latch *dcwb_latch = &pipeline->dcwb_latch;
  struct vr4300_bus_request *request = &exdc_latch->request;

  int datashift = (8 - request->size) << 3;
  int64_t sdata = request->data;

  // Shall we sign extend?
  sdata = (int64_t) ((uint64_t) sdata << datashift) >> datashift;
  dcwb_latch->result |= (sdata & request->dqm) << request->postshift;

  vr4300_cycle_slow_ex(vr4300);
}

// LUT of stages for fault handling.
typedef void (*pipeline_function)(struct vr4300 *vr4300);
static const pipeline_function pipeline_function_lut[] = {
  vr4300_cycle_slow_wb,
  vr4300_cycle_slow_dc,
  vr4300_cycle_slow_ex,
  vr4300_cycle_slow_rf,
  vr4300_cycle_slow_ic,
  vr4300_cycle_slow_ex_fixdc,
  vr4300_cycle_busywait,
};

// Advances the processor pipeline by one pclock.
void vr4300_cycle(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  // Increment counters.
  vr4300->regs[VR4300_CP0_REGISTER_COUNT] += ++(vr4300->cycles) & 0x1;

  if ((uint32_t) vr4300->regs[VR4300_CP0_REGISTER_COUNT] ==
    (uint32_t) vr4300->regs[VR4300_CP0_REGISTER_COMPARE])
    vr4300->regs[VR4300_CP0_REGISTER_CAUSE] |= 0x8000;

  // We're stalling for something...
  if (pipeline->cycles_to_stall > 0) {
    pipeline->cycles_to_stall--;
    return;
  }

  // Ordinarily, we would need to check every pipeline stage to see if it is
  // aborted, and conditionally not execute it. Since faults are rare, we'll
  // only bother checking for aborted stages when we know they can be present.
  if (pipeline->fault_present + vr4300->regs[PIPELINE_CYCLE_TYPE]) {
    pipeline_function_lut[vr4300->regs[PIPELINE_CYCLE_TYPE]](vr4300);
    return;
  }

  if (vr4300_wb_stage(vr4300))
    return;

  if (vr4300_dc_stage(vr4300))
    return;

  if (vr4300_ex_stage(vr4300))
    return;

  if (vr4300_rf_stage(vr4300))
    return;

  if (vr4300_ic_stage(vr4300))
    return;
}

// Initializes the pipeline with default values.
void vr4300_pipeline_init(struct vr4300_pipeline *pipeline) {
  pipeline->icrf_latch.segment = get_default_segment();
  pipeline->exdc_latch.segment = get_default_segment();
}

