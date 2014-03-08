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

const char *vr4300_fault_mnemonics[NUM_VR4300_FAULTS] = {
#define X(fault) #fault,
#include "vr4300/fault.md"
#undef X
};

// Sets attributes common to all exceptions.
static void vr4300_common_exceptions(struct vr4300_pipeline *pipeline) {
  pipeline->icrf_latch.segment = get_default_segment();

  pipeline->exception_history = 0;
  pipeline->fault_present = true;
  pipeline->cycles_to_stall = 2;
  pipeline->skip_stages = 0;
}

// Sets attributes common to all interlocks.
static void vr4300_common_interlocks(struct vr4300_pipeline *pipeline,
  unsigned cycles_to_stall, unsigned skip_stages) {
  pipeline->cycles_to_stall = cycles_to_stall;
  pipeline->skip_stages = skip_stages;
}

// Raise a fault that originated in the DC stage.
static void vr4300_dc_fault(struct vr4300_pipeline *pipeline,
  enum vr4300_fault_id fault) {
  vr4300_common_exceptions(pipeline);
  pipeline->exdc_latch.common.fault = fault;
  pipeline->rfex_latch.common.fault = fault;
  pipeline->icrf_latch.common.fault = fault;
}

// IADE: Instruction address error exception
void VR4300_IADE(unused(struct vr4300 *vr4300)) {
  abort();
}

// UNC: Uncached read interlock.
void VR4300_UNC(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_icrf_latch *icrf_latch = &pipeline->icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &pipeline->rfex_latch;
  const struct segment *segment = icrf_latch->segment;
  uint64_t address;

  vr4300_common_interlocks(pipeline, ~0, 4);

  address = icrf_latch->common.pc - segment->offset;
  bus_read_word(vr4300->bus, address, &rfex_latch->iw);
  pipeline->cycles_to_stall = 50;
}

// RST: External reset exception.
void VR4300_RST(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_cp0 *cp0 = &vr4300->cp0;

  // Prepare pipeline for restart.
  vr4300->pipeline.icrf_latch.pc = 0xFFFFFFFFBFC00000ULL;
  vr4300_dc_fault(pipeline, VR4300_FAULT_RST);

  // Cold reset exception.
  if (vr4300->signals & VR4300_SIGNAL_COLDRESET) {
    vr4300->signals &= ~VR4300_SIGNAL_COLDRESET;

    cp0->regs[VR4300_CP0_REGISTER_STATUS] &= ~0x08300000ULL;
    cp0->regs[VR4300_CP0_REGISTER_CONFIG] &= ~0xFFFF7FF0ULL;

    cp0->regs[VR4300_CP0_REGISTER_STATUS] |= 0x00400004ULL;
    cp0->regs[VR4300_CP0_REGISTER_CONFIG] |= 0x7006E460ULL;

    cp0->regs[VR4300_CP0_REGISTER_RANDOM] = 31;
  }

  // Soft reset exception.
  else
    abort();
}

