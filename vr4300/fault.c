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
#include "vr4300/dcache.h"
#include "vr4300/fault.h"
#include "vr4300/icache.h"
#include "vr4300/pipeline.h"

// Currently used a fixed value...
#define DCACHE_ACCESS_DELAY (48 - 2)
#define ICACHE_ACCESS_DELAY (52 - 2)
#define MEMORY_WORD_DELAY 40

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
  struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  struct vr4300_bus_request *request = &exdc_latch->request;
  const struct segment *segment = exdc_latch->segment;

  uint64_t vaddr = request->vaddr;
  uint32_t paddr = request->paddr;
  struct vr4300_dcache_line *line;
  uint32_t data[4];
  unsigned i;

  if (!segment->cached) {

    // Service a read.
    if (exdc_latch->request.type == VR4300_BUS_REQUEST_READ) {
      uint32_t hiword, loword;
      int64_t sdata;

      bus_read_word(vr4300->bus, paddr & ~0x3ULL, &hiword);

      if (!request->two_words) {
        unsigned rshiftamt = (4 - request->size) << 3;
        unsigned lshiftamt = (paddr & 0x3) << 3;

        sdata = (int32_t) (hiword << lshiftamt) >> rshiftamt;
      }

      else {
        unsigned rshiftamt = (8 - request->size) << 3;
        unsigned lshiftamt = (paddr & 0x7) << 3;

        bus_read_word(vr4300->bus, (paddr & ~0x7ULL) + 4, &loword);
        sdata = (int64_t) (((uint64_t) hiword | loword) << lshiftamt) >>
          rshiftamt;
      }

      dcwb_latch->result |= sdata << request->postshift;
    }

    // Service a write.
    else {
      uint64_t data = request->data;
      uint64_t dqm = request->dqm;

      if (request->size > 4) {
        bus_write_word(vr4300->bus, paddr, data >> 32, dqm >> 32);
        paddr += 4;
      }
 
      bus_write_word(vr4300->bus, paddr, data, dqm);
    }

    vr4300_common_interlocks(vr4300, MEMORY_WORD_DELAY, 2);
    return;
  }

  // Cached accesses require us to potentially flush the old line.
  // In addition to that, we also need to pull in the next one.
  if ((line = vr4300_dcache_should_flush_line(
    &vr4300->dcache, vaddr)) != NULL) {
    uint32_t bus_address;

    bus_address = vr4300_dcache_get_tag(line);
    memcpy(data, line->data, sizeof(data));

    for (i = 0; i < 4; i++)
      bus_write_word(vr4300->bus, bus_address + i * 4, data[i], ~0);
  }

  // Raise interlock condition, get virtual address.
  vr4300_common_interlocks(vr4300, DCACHE_ACCESS_DELAY, 1);
  paddr &= ~0xF;

  // Fill the cache line.
  for (i = 0; i < 4; i++)
    bus_read_word(vr4300->bus, paddr + i * 4, data+ i);

  vr4300_dcache_fill(&vr4300->dcache, vaddr, paddr, data);
}

// DCM: Data cache miss interlock.
void VR4300_DCM(struct vr4300 *vr4300) {
  vr4300_common_interlocks(vr4300, MEMORY_WORD_DELAY, 6);
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
  uint64_t vaddr = icrf_latch->common.pc - segment->offset;
  unsigned delay;

  if (!segment->cached) {
    bus_read_word(vr4300->bus, vaddr, &rfex_latch->iw);
    delay = MEMORY_WORD_DELAY;
  }

  else {
    uint32_t line[8];
    uint32_t paddr;
    unsigned i;

    paddr = vaddr & ~0x1C;

    // Fill the cache line.
    for (i = 0; i < 8; i ++)
      bus_read_word(vr4300->bus, paddr + i * 4, line + i);

    memcpy(&rfex_latch->iw, line + (vaddr >> 2 & 0x7), sizeof(rfex_latch->iw));
    vr4300_icache_fill(&vr4300->icache, icrf_latch->common.pc, paddr, line);
    delay = ICACHE_ACCESS_DELAY;
  }

  vr4300_common_interlocks(vr4300, delay, 4);
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

