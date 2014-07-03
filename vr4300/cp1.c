//
// vr4300/cp1.c: VR4300 floating point unit coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/cp0.h"
#include "vr4300/cp1.h"
#include "vr4300/cpu.h"

static bool vr4300_cp1_usable(const struct vr4300 *vr4300);

//
// Determines if the coprocessor was used yet.
//
bool vr4300_cp1_usable(const struct vr4300 *vr4300) {
  return (vr4300->regs[VR4300_CP0_REGISTER_STATUS] & 0x20000000) != 0;
}

//
// CFC1
//
int VR4300_CFC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_RT(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  exdc_latch->result = (int32_t) 0;
  exdc_latch->dest = dest;
  return 0;
}

//
// CTC1
//
int VR4300_CTC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  return 0;
}

//
// LDC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LDC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = VR4300_REGISTER_CP1_0 + GET_RT(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  exdc_latch->request.address = rs + (int16_t) iw;
  exdc_latch->request.dqm = ~0ULL;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.preshift = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = 8;

  exdc_latch->dest = dest;
  exdc_latch->result = 0;
  return 0;
}

//
// LWC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LWC1(struct vr4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  uint32_t iw = rfex_latch->iw;
  uint64_t address = (rs + (int16_t) iw);
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  uint64_t result = 0;
  unsigned dest = VR4300_REGISTER_CP1_0 + GET_RT(iw);
  unsigned postshift = 0;

  if (!(status & 0x04000000)) {
    result = dest & 0x1
      ? vr4300->regs[dest & ~0x1] & 0x00000000FFFFFFFFULL
      : vr4300->regs[dest & ~0x1] & 0xFFFFFFFF00000000ULL;

    postshift = 32;
    dest &= ~0x1;
  }

  exdc_latch->request.address = address;
  exdc_latch->request.dqm = ~0U;
  exdc_latch->request.postshift = postshift;
  exdc_latch->request.preshift = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = 4;

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// MTC1
//
int VR4300_MTC1(struct vr4300 *vr4300, uint64_t unused(rs), uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FS(iw) + VR4300_REGISTER_CP1_0;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  // TODO/FIXME: Err... forward here?
  if (!(status & 0x04000000)) {
    uint64_t fs = vr4300->regs[dest & ~0x1];

    if (dest & 0x1)
      vr4300->regs[dest & ~0x1] = ((uint32_t) fs) | (rt << 32);
    else
      vr4300->regs[dest & ~0x1] = (fs << 32) | ((uint32_t) rt);

    return 0;
  }

  // TODO/FIXME: Sign extend, or...?
  vr4300->regs[dest] = (int32_t) rt;
  return 0;
}

// Initializes the coprocessor.
void vr4300_cp1_init(struct vr4300 *vr4300) {
  memset(vr4300->regs + VR4300_REGISTER_CP1_0, 0,
    sizeof(*vr4300->regs) * NUM_VR4300_CP1_REGISTERS);
}

