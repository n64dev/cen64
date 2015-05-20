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

typedef void (*pipeline_function)(struct vr4300 *vr4300);

cen64_cold static void vr4300_cycle_slow_wb(struct vr4300 *vr4300);
cen64_cold static void vr4300_cycle_slow_dc(struct vr4300 *vr4300);
static void vr4300_cycle_slow_ex(struct vr4300 *vr4300);
static void vr4300_cycle_slow_rf(struct vr4300 *vr4300);
static void vr4300_cycle_slow_ic(struct vr4300 *vr4300);
static void vr4300_cycle_busywait(struct vr4300 *vr4300);

// Prints out instructions and their virtual address as they are executed.
// Note: Some of these instructions _may_ be speculative and killed later...
//#define PRINT_EXEC

// Instruction cache stage.
cen64_flatten static void vr4300_ic_stage(struct vr4300 *vr4300) {
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
  icrf_latch->common.pc = pc;
  icrf_latch->pc = pc + 4;

  // If decoding of prior instruction indicates this is a BD slot...
  icrf_latch->common.cause_data = (opcode->flags & OPCODE_INFO_BRANCH);

  // Look up the segment that we're in.
  if ((pc - segment->start) >= segment->length) {
    uint32_t cp0_status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

    if (unlikely((segment = get_segment(pc, cp0_status)) == NULL))
      VR4300_IADE(vr4300);

    // Next stage gets killed either way, so we can safely
    // latch a faulty segment and not worry about it.
    icrf_latch->segment = segment;
  }
}

// Register fetch and decode stage.
static int vr4300_rf_stage(struct vr4300 *vr4300) {
  const struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;

  const struct segment *segment = icrf_latch->segment;
  const struct vr4300_icache_line *line;
  uint64_t vaddr = icrf_latch->common.pc;
  uint32_t paddr;
  bool cached;

  rfex_latch->common = icrf_latch->common;

  // If we're in a mapped region, do a TLB translation.
  paddr = vaddr - segment->offset;
  cached = segment->cached;

  if (segment->mapped) {
    unsigned asid = vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI] & 0xFF;
    unsigned select, tlb_miss, index;
    uint32_t page_mask;

    tlb_miss = tlb_probe(&vr4300->cp0.tlb, vaddr, asid, &index);
    page_mask = vr4300->cp0.page_mask[index];
    select = ((page_mask + 1) & vaddr) != 0;

    if (unlikely(tlb_miss || !(vr4300->cp0.state[index][select] & 2))) {
      VR4300_ITLB(vr4300, tlb_miss);
      return 1;
    }

    cached = (vr4300->cp0.state[index][select] & 0x38) != 0x10;
    paddr = (vr4300->cp0.pfn[index][select]) | (vaddr & page_mask);
  }

  // If not cached or we miss in the IC, it's an ICB.
  line = vr4300_icache_probe(&vr4300->icache, vaddr, paddr);

  if (!(line && cached)) {
    rfex_latch->paddr = paddr;
    rfex_latch->cached = cached;

    VR4300_ICB(vr4300);
    return 1;
  }

  memcpy(&rfex_latch->iw, line->data + (paddr & 0x1C),
    sizeof(rfex_latch->iw));

  return 0;
}

// Execution stage.
static int vr4300_ex_stage(struct vr4300 *vr4300) {
  const struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  const struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t cp0_status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  unsigned rs, rt, rslutidx, rtlutidx;
  uint64_t rs_reg, rt_reg, temp;
  uint32_t flags, iw;

  exdc_latch->common = rfex_latch->common;
  flags = rfex_latch->opcode.flags;
  iw = rfex_latch->iw;

  rs = GET_RS(iw);
  rt = GET_RT(iw);

  if (flags & OPCODE_INFO_FPU) {
    cen64_align(static const unsigned fpu_select_lut[2], 8) = {21, 11};
    unsigned fr;

    // Dealing with FPU state, is CP1 usable?
    if (unlikely(!(cp0_status & 0x20000000U))) {
      VR4300_CPU(vr4300);
      return 1;
    }

    // Check if one of the sources is an FPU register. Furthermore,
    // if Status.FR bit is clear, we depend on even registers only.
    fr = (cp0_status >> 26 & 0x1) ^ 0x1;
    rtlutidx = flags & 0x2;
    rslutidx = flags & 0x1;

    rs = (iw >> fpu_select_lut[rslutidx] & 0x1F) | (rslutidx << 6);
    rt |= (rtlutidx << 5);

    rs &= ~((rslutidx) & fr);
    rt &= ~((rtlutidx >> 1) & fr);
  }

  // Check to see if we should hold off execution due to a LDI.
  // MIPS compilers generally optimize this out, so flag as unlikely.
  if (unlikely(exdc_latch->request.type == VR4300_BUS_REQUEST_READ && (
    ((exdc_latch->dest == rs) && (flags & OPCODE_INFO_NEEDRS)) ||
    ((exdc_latch->dest == rt) && (flags & OPCODE_INFO_NEEDRT))))) {
    VR4300_LDI(vr4300);
    return 1;
  }

  // No LDI, so we just need to forward results from DC/WB.
  // This is done to preserve RF state and fwd without branching.
  temp = vr4300->regs[dcwb_latch->dest];
  vr4300->regs[dcwb_latch->dest] = dcwb_latch->result;
  vr4300->regs[VR4300_REGISTER_R0] = 0x0000000000000000ULL;

  rs_reg = vr4300->regs[rs];
  rt_reg = vr4300->regs[rt];

  vr4300->regs[dcwb_latch->dest] = temp;

  // Finally, execute the instruction.
#ifdef PRINT_EXEC
  debug("%.16llX: %s\n", (unsigned long long) rfex_latch->common.pc,
    vr4300_opcode_mnemonics[rfex_latch->opcode.id]);
#endif

  exdc_latch->dest = VR4300_REGISTER_R0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_NONE;
  return vr4300_function_table[rfex_latch->opcode.id](
    vr4300, iw, rs_reg, rt_reg);
}

// Data cache fetch stage.
static int vr4300_dc_stage(struct vr4300 *vr4300) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;

  uint32_t cp0_status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint32_t cp0_cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];
  const struct segment *segment = exdc_latch->segment;
  bool cached;

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
  if (unlikely(cp0_cause & cp0_status & 0xFF00 &&
    (((cp0_status ^ 6) & 0x7) == 0x7))) {
    VR4300_INTR(vr4300);
    return 1;
  }

  // Look up the segment that we're in.
  if (exdc_latch->request.type != VR4300_BUS_REQUEST_NONE) {
    struct vr4300_bus_request *request = &exdc_latch->request;
    uint64_t vaddr = exdc_latch->request.vaddr;
    struct vr4300_dcache_line *line;
    uint32_t paddr;

    if ((vaddr - segment->start) >= segment->length) {
      if (unlikely((segment = get_segment(vaddr, cp0_status)) == NULL)) {
        VR4300_DADE(vr4300);
        return 1;
      }

      exdc_latch->segment = segment;
    }

    // If we're in a mapped region, do a TLB translation.
    paddr = vaddr - segment->offset;
    cached = segment->cached;

    if (segment->mapped) {
      unsigned asid = vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI] & 0xFF;
      unsigned select, tlb_inv, tlb_miss, tlb_mod, index;
      uint32_t page_mask;

      tlb_miss = tlb_probe(&vr4300->cp0.tlb, vaddr, asid, &index);
      page_mask = vr4300->cp0.page_mask[index];
      select = ((page_mask + 1) & vaddr) != 0;

      tlb_inv = !(vr4300->cp0.state[index][select] & 2);

      tlb_mod = !(vr4300->cp0.state[index][select] & 4) &&
        request->type == VR4300_BUS_REQUEST_WRITE;

      if (unlikely(tlb_miss | tlb_inv | tlb_mod)) {
        VR4300_DTLB(vr4300, tlb_miss, tlb_inv, tlb_mod);
        return 1;
      }

      cached = ((vr4300->cp0.state[index][select] & 0x38) != 0x10);
      paddr = (vr4300->cp0.pfn[index][select]) | (vaddr & page_mask);
    }

    // Check to see if we should raise a WAT exception.
    if (unlikely((paddr & ~0x80000007U) == (vr4300->regs[
      VR4300_CP0_REGISTER_WATCHLO] & ~0x80000007U))) {

      // We hit the address, just check load/store.
      // TODO: Postposted if EXL bit is set.
      if (vr4300->regs[VR4300_CP0_REGISTER_WATCHLO] & request->type & 0x3) {
        VR4300_WAT(vr4300);
        return 1;
      }
    }

    // If not cached or we miss in the DC, it's an DCB.
    if (likely(exdc_latch->request.type < VR4300_BUS_REQUEST_CACHE)) {
      uint64_t dword, rtemp, wtemp, wdqm;
      unsigned shiftamt, rshiftamt, lshiftamt;
      uint32_t s_paddr;

      line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr);

      if (!(line && cached)) {
        request->paddr = paddr;
        exdc_latch->cached = cached;

        VR4300_DCM(vr4300);
        return 1;
      }

      s_paddr = paddr << 3;
      paddr &= 0x8;

      // Pull out the cache line data, mux stuff around
      // to produce the output/update the cache line.
      memcpy(&dword, line->data + paddr, sizeof(dword));

      shiftamt = (s_paddr ^ (WORD_ADDR_XOR << 3)) & request->access_type;
      rshiftamt = (8 - request->size) << 3;
      lshiftamt = (s_paddr & (0x7 << 3));

      wdqm = request->wdqm << shiftamt;
      wtemp = (request->data << shiftamt) & wdqm;
      rtemp = ((int64_t) (dword << lshiftamt)
        >> rshiftamt) & request->data;

      dword = (dword & ~wdqm) | wtemp;
      dcwb_latch->result |= rtemp << request->postshift;
      memcpy(line->data + paddr, &dword, sizeof(dword));

      // We need to mark the line dirty if it's write.
      // Fortunately, metadata & 0x2 == dirty, and
      // metadata 0x1 == valid. Our requests values are
      // read (0x1) and write (0x2), so we can just do
      // a simple OR here without impacting anything.
      line->metadata |= exdc_latch->request.type;
    }

    // Not a load/store, so execute cache operation.
    else
      return request->cacheop(vr4300, vaddr, paddr);
  }

  return 0;
}

// Writeback stage.
static int vr4300_wb_stage(struct vr4300 *vr4300) {
  const struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;

  vr4300->regs[dcwb_latch->dest] = dcwb_latch->result;
  return 0;
}

// Advances the processor pipeline by one pclock.
// May have exceptions, so check for aborted stages.
void vr4300_cycle_slow_wb(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_dcwb_latch *dcwb_latch = &pipeline->dcwb_latch;

  // If we haven't had exceptions for at least a
  // full pipeline's length, switch back to fast mode.
  if (pipeline->exception_history++ > 3)
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

  else {
    dcwb_latch->common = exdc_latch->common;
    dcwb_latch->dest = 0;
  }

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

  else {
    exdc_latch->common = rfex_latch->common;
    exdc_latch->dest = 0;
  }

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
  vr4300->pipeline.icrf_latch.common.fault = VR4300_FAULT_NONE;

  vr4300->regs[PIPELINE_CYCLE_TYPE] = 0;

  // If IADE is raised, it'll reset the PIPELINE_CYCLE_TYPE
  // so we can aggressively force it back to zero first.
  vr4300_ic_stage(vr4300);
}

// Special-cased busy wait handler.
void vr4300_cycle_busywait(struct vr4300 *vr4300) {
  uint32_t cp0_status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint32_t cp0_cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];

  // Check if the busy wait period is over (due to an interrupt condition).
  if (unlikely(cp0_cause & cp0_status & 0xFF00)
    && (cp0_status & 0x1) && !(cp0_status & 0x6)) {
    //debug("Busy wait done @ %llu cycles\n", vr4300->cycles);

    VR4300_INTR(vr4300);
  }
}

// LUT of stages for fault handling.
cen64_align(static const pipeline_function
  pipeline_function_lut[], CACHE_LINE_SIZE) = {
  vr4300_cycle_slow_wb,
  vr4300_cycle_slow_dc,
  vr4300_cycle_slow_ex,
  vr4300_cycle_slow_rf,
  vr4300_cycle_slow_ic,

  vr4300_cycle_busywait,
  VR4300_DCB,
};

// Advances the processor pipeline by one pclock.
void vr4300_cycle(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  // Increment counters.
  vr4300->regs[VR4300_CP0_REGISTER_COUNT]++;

  if ((uint32_t) (vr4300->regs[VR4300_CP0_REGISTER_COUNT] >> 1) ==
    (uint32_t) vr4300->regs[VR4300_CP0_REGISTER_COMPARE])
    vr4300->regs[VR4300_CP0_REGISTER_CAUSE] |= 0x8000;

  // We're stalling for something...
  if (pipeline->cycles_to_stall > 0)
    pipeline->cycles_to_stall--;

  // Ordinarily, we would need to check every pipeline stage to see if it is
  // aborted, and conditionally not execute it. Since faults are rare, we'll
  // only bother checking for aborted stages when we know they can be present.
  else if (pipeline->fault_present + vr4300->regs[PIPELINE_CYCLE_TYPE])
    pipeline_function_lut[vr4300->regs[PIPELINE_CYCLE_TYPE]](vr4300);

  else {
    if (vr4300_wb_stage(vr4300))
      return;

    if (vr4300_dc_stage(vr4300))
      return;

    if (vr4300_ex_stage(vr4300))
      return;

    if (vr4300_rf_stage(vr4300))
      return;

    vr4300_ic_stage(vr4300);
  }
}

// Collects additional information about the pipeline each cycle.
void vr4300_cycle_extra(struct vr4300 *vr4300, struct vr4300_stats *stats) {
  struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;

  // Collect information for CPI.
  stats->executed_instructions +=
    !dcwb_latch->common.fault &&
    !vr4300->pipeline.cycles_to_stall;

  stats->total_cycles++;

  // Collect information about executed instructions.
  stats->opcode_counts[rfex_latch->opcode.id]++;
}

// Initializes the pipeline with default values.
void vr4300_pipeline_init(struct vr4300_pipeline *pipeline) {
  pipeline->icrf_latch.segment = get_default_segment();
  pipeline->exdc_latch.segment = get_default_segment();
}

