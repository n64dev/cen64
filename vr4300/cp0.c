//
// vr4300/cp0.c: VR4300 system control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/cp0.h"
#include "vr4300/cpu.h"

//
// ERET
//
int VR4300_ERET(struct vr4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  int32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  if (status & 0x4) {
    icrf_latch->pc = vr4300->regs[VR4300_CP0_REGISTER_ERROREPC];
    status &= ~0x4;
  }

  else {
    icrf_latch->pc = vr4300->regs[VR4300_CP0_REGISTER_EPC];
    status &= ~0x2;
  }

  // Until we delay CP0 writes, we have to kill ourselves
  // to prevent squashing this instruction the next cycle.
  exdc_latch->common.fault = ~0;
  icrf_latch->common.fault = ~0;

  pipeline->exception_history = 0;
  pipeline->fault_present = true;
  pipeline->skip_stages = 0;

  vr4300->regs[VR4300_CP0_REGISTER_STATUS] = status;
  // vr4300->llbit = 0;
  return 0;
}

//
// MFC0
// TODO/FIXME: Combine with MFC{1,2}?
//
int VR4300_MFC0(struct vr4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned src = GET_RD(iw) + 32;
  unsigned dest = GET_RT(iw);

  exdc_latch->result = (int32_t) vr4300->regs[src];
  exdc_latch->dest = dest;
  return 0;
}

//
// MTC0
//
int VR4300_MTC0(struct vr4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_RD(iw) + 32;

  if (dest == VR4300_CP0_REGISTER_COMPARE)
    vr4300->regs[VR4300_CP0_REGISTER_CAUSE] &= ~0x8000;

  // TODO/FIXME: Sign extend, or...?
  // Would make sense for EPC, etc.
  //exdc_latch->result = (int32_t) rt;
  //exdc_latch->dest = dest;
  vr4300->regs[dest] = (int32_t) rt;
  return 0;
}

// Initializes the coprocessor.
void vr4300_cp0_init(struct vr4300 *vr4300) {
  memset(vr4300->regs + VR4300_REGISTER_CP0_0, 0,
    sizeof(*vr4300->regs) * NUM_VR4300_CP0_REGISTERS);
}

