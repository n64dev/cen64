//
// vr4300/fault.c: VR4300 fault management.
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
#include "vr4300/fault.h"
#include "vr4300/pipeline.h"

// Currently used a fixed value...
#define MEMORY_CODE_CYCLE_DELAY 50
#define MEMORY_DATA_CYCLE_DELAY 5
#define ICACHE_ACCESS_DELAY 50

const char *vr4300_fault_mnemonics[NUM_VR4300_FAULTS] = {
#define X(fault) #fault,
#include "vr4300/fault.md"
#undef X
};

// Sets attributes common to all exceptions.
static void vr4300_common_exceptions(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  pipeline->icrf_latch.segment = get_default_segment();
  pipeline->exdc_latch.segment = get_default_segment();

  vr4300->regs[PIPELINE_CYCLE_TYPE] = 0;
  pipeline->exception_history = 0;
  pipeline->fault_present = true;
  pipeline->cycles_to_stall = 2;
}

// Sets attributes common to all interlocks.
static void vr4300_common_interlocks(struct vr4300 *vr4300,
  unsigned cycles_to_stall, unsigned skip_stages) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  pipeline->cycles_to_stall = cycles_to_stall;
  vr4300->regs[PIPELINE_CYCLE_TYPE] = skip_stages;
}

// Raise a fault that originated in the DC stage.
static void vr4300_dc_fault(struct vr4300 *vr4300, enum vr4300_fault_id fault) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  vr4300_common_exceptions(vr4300);
  pipeline->dcwb_latch.common.fault = fault;
  pipeline->exdc_latch.common.fault = fault;
  pipeline->rfex_latch.common.fault = fault;
  pipeline->icrf_latch.common.fault = fault;
}

// Raise a fault that originated in the EX stage.
static void vr4300_ex_fault(struct vr4300 *vr4300, enum vr4300_fault_id fault) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  vr4300_common_exceptions(vr4300);
  pipeline->exdc_latch.common.fault = fault;
  pipeline->rfex_latch.common.fault = fault;
  pipeline->icrf_latch.common.fault = fault;
}

// CPU: Coprocessor unusable exception.
void VR4300_CPU(unused(struct vr4300 *vr4300)) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_latch *common = &pipeline->exdc_latch.common;

  bool in_bd_slot = common->cause_data >> 31;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint32_t cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];
  uint64_t epc = vr4300->regs[VR4300_CP0_REGISTER_EPC];

  // Record branch delay slot?
  if (!(status & 0x2)) {
    if (in_bd_slot) {
      cause |= 0x80000000U;
      epc = common->pc - 4;
    }

    else {
      cause &= ~0x80000000U;
      epc = common->pc;
    }
  }

  else assert(0);

  // TODO/FIXME: Check for XTLB/TLB miss exceptions.
  // For now, we're just hard-coding the vector offset.

  // Prepare pipeline for restart.
  vr4300->regs[VR4300_CP0_REGISTER_STATUS] = status | 0x2;
  vr4300->regs[VR4300_CP0_REGISTER_CAUSE] = (cause & ~0xFF) | (1 << 28) | 0x2C;
  vr4300->regs[VR4300_CP0_REGISTER_EPC] = epc;

  vr4300_ex_fault(vr4300, VR4300_FAULT_CPU);
  vr4300->pipeline.icrf_latch.pc = (status & 0x400000)
    ? 0xFFFFFFFFBFC00280ULL
    : 0xFFFFFFFF80000080ULL;
}

// DADE: Data address error exception.
void VR4300_DADE(unused(struct vr4300 *vr4300)) {
  abort(); // Hammertime!
}

// DCB: Data cache busy interlock.
void VR4300_DCB(struct vr4300 *vr4300) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  struct vr4300_bus_request *request = &exdc_latch->request;
  uint32_t paddr = request->address;

  // Service a read.
  if (exdc_latch->request.type == VR4300_BUS_REQUEST_READ) {
    uint32_t word;

    vr4300_common_interlocks(vr4300, MEMORY_DATA_CYCLE_DELAY, 5);
    bus_read_word(vr4300->bus, paddr & ~0x3ULL, &word);

    if (!request->two_words) {
      unsigned rshiftamt = (4 - request->size) << 3;
      unsigned lshiftamt = (paddr & 0x3) << 3;

      request->data = ((int64_t) (word << lshiftamt)) >> rshiftamt;
    }

    else {
      unsigned rshiftamt = (8 - request->size) << 3;
      unsigned lshiftamt = (paddr & 0x7) << 3;

      request->data = (uint64_t) word << 32;
      bus_read_word(vr4300->bus, (paddr & ~0x7ULL) + 4, &word);
      request->data = (int64_t) ((request->data | word) << lshiftamt) >>
        rshiftamt;
    }
  }

  // Service a write.
  else {
    uint64_t data = request->data;
    uint64_t dqm = request->dqm;

    vr4300_common_interlocks(vr4300, MEMORY_DATA_CYCLE_DELAY, 2);

    if (request->size > 4) {
      bus_write_word(vr4300->bus, paddr, data >> 32, dqm >> 32);
      paddr += 4;
    }

    bus_write_word(vr4300->bus, paddr, data, dqm);
  }
}

// DCM: Data cache miss interlock.
void VR4300_DCM(struct vr4300 *vr4300) {
  vr4300_common_interlocks(vr4300, MEMORY_DATA_CYCLE_DELAY, 7);
}

// IADE: Instruction address error exception.
void VR4300_IADE(unused(struct vr4300 *vr4300)) {
  abort(); // Hammertime!
}

// ICB: Instruction cache busy interlock.
void VR4300_ICB(unused(struct vr4300 *vr4300)) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  const struct segment *segment = icrf_latch->segment;

  uint32_t line[8];
  uint32_t paddr;
  unsigned i;

  // Raise interlock condition, get virtual address.
  vr4300_common_interlocks(vr4300, ICACHE_ACCESS_DELAY, 4);
  paddr = (icrf_latch->common.pc - segment->offset) & ~0x1C;

  // Fill the cache line.
  for (i = 0; i < 8; i ++)
    bus_read_word(vr4300->bus, paddr + i * 4, line + i);

  // Fill the line, read the first word.
  i = (icrf_latch->common.pc - segment->offset) & 0x1C;

  memcpy(&rfex_latch->iw, line + (i >> 2), sizeof(rfex_latch->iw));
  vr4300_icache_fill(&vr4300->icache, icrf_latch->common.pc,
    paddr, line);
}

// INTR: Interrupt exception.
void VR4300_INTR(unused(struct vr4300 *vr4300)) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_latch *common = &pipeline->dcwb_latch.common;

  bool in_bd_slot = common->cause_data >> 31;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint32_t cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];
  uint64_t epc = vr4300->regs[VR4300_CP0_REGISTER_EPC];

  // Record branch delay slot?
  if (!(status & 0x2)) {
    if (in_bd_slot) {
      cause |= 0x80000000U;
      epc = common->pc - 4;
    }

    else {
      cause &= ~0x80000000U;
      epc = common->pc;
    }
  }

  else assert(0);

  // TODO/FIXME: Check for XTLB/TLB miss exceptions.
  // For now, we're just hard-coding the vector offset.

  // Prepare pipeline for restart.
  vr4300->regs[VR4300_CP0_REGISTER_STATUS] = status | 0x2;
  vr4300->regs[VR4300_CP0_REGISTER_CAUSE] = cause & ~0xFF;
  vr4300->regs[VR4300_CP0_REGISTER_EPC] = epc;

  vr4300_dc_fault(vr4300, VR4300_FAULT_INTR);
  vr4300->pipeline.icrf_latch.pc = (status & 0x400000)
    ? 0xFFFFFFFFBFC00280ULL
    : 0xFFFFFFFF80000080ULL;
}

// LDI: Load delay interlock.
void VR4300_LDI(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_exdc_latch *exdc_latch = &pipeline->exdc_latch;

  // We'll do EX again, but clear the 'busy' flag.
  exdc_latch->request.type = VR4300_BUS_REQUEST_NONE;
  vr4300_common_interlocks(vr4300, 0, 2);
}

// RST: External reset exception.
void VR4300_RST(struct vr4300 *vr4300) {
  vr4300->pipeline.icrf_latch.pc = 0xFFFFFFFFBFC00000ULL;
  vr4300_dc_fault(vr4300, VR4300_FAULT_RST);

  // Cold reset exception.
  if (vr4300->signals & VR4300_SIGNAL_COLDRESET) {
    vr4300->signals &= ~VR4300_SIGNAL_COLDRESET;

    vr4300->regs[VR4300_CP0_REGISTER_STATUS] &= ~0x08300000ULL;
    vr4300->regs[VR4300_CP0_REGISTER_CONFIG] &= ~0xFFFF7FF0ULL;

    vr4300->regs[VR4300_CP0_REGISTER_STATUS] |= 0x00400004ULL;
    vr4300->regs[VR4300_CP0_REGISTER_CONFIG] |= 0x7006E460ULL;

    vr4300->regs[VR4300_CP0_REGISTER_RANDOM] = 31;
  }

  // Soft reset exception.
  else
    abort();
}

// UNC: Uncached read interlock.
void VR4300_UNC(struct vr4300 *vr4300) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  const struct segment *segment = icrf_latch->segment;
  uint64_t address;

  vr4300_common_interlocks(vr4300, MEMORY_CODE_CYCLE_DELAY, 4);

  address = icrf_latch->common.pc - segment->offset;
  bus_read_word(vr4300->bus, address, &rfex_latch->iw);
}

