//
// vr4300/cp0.h: VR4300 system control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_cp0_h__
#define __vr4300_cp0_h__
#include "common.h"
#include "tlb/tlb.h"

struct vr4300;

struct vr4300_cp0 {
  struct cen64_tlb tlb;

  uint32_t pfn[32][2];
  uint8_t state[32][2];
};

// Registers list.
enum vr4300_cp0_register {
  VR4300_CP0_REGISTER_INDEX = 32,
  VR4300_CP0_REGISTER_RANDOM = 33,
  VR4300_CP0_REGISTER_ENTRYLO0 = 34,
  VR4300_CP0_REGISTER_ENTRYLO1 = 35,
  VR4300_CP0_REGISTER_CONTEXT = 36,
  VR4300_CP0_REGISTER_PAGEMASK = 37,
  VR4300_CP0_REGISTER_WIRED = 38,
  VR4300_CP0_REGISTER_BADVADDR = 40,
  VR4300_CP0_REGISTER_COUNT = 41,
  VR4300_CP0_REGISTER_ENTRYHI = 42,
  VR4300_CP0_REGISTER_COMPARE = 43,
  VR4300_CP0_REGISTER_STATUS = 44,
  VR4300_CP0_REGISTER_CAUSE = 45,
  VR4300_CP0_REGISTER_EPC = 46,
  VR4300_CP0_REGISTER_PRID = 47,
  VR4300_CP0_REGISTER_CONFIG = 48,
  VR4300_CP0_REGISTER_LLADDR = 49,
  VR4300_CP0_REGISTER_WATCHLO = 50,
  VR4300_CP0_REGISTER_WATCHHI = 51,
  VR4300_CP0_REGISTER_XCONTEXT = 52,
  VR4300_CP0_REGISTER_PARITYERROR = 58,
  VR4300_CP0_REGISTER_CACHEERR = 59,
  VR4300_CP0_REGISTER_TAGLO = 60,
  VR4300_CP0_REGISTER_TAGHI = 61,
  VR4300_CP0_REGISTER_ERROREPC = 62,
  NUM_VR4300_CP0_REGISTERS = 32,
};

int VR4300_DMFC0(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);
int VR4300_DMTC0(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);
int VR4300_ERET(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);
int VR4300_MFC0(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);
int VR4300_MTC0(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);
int VR4300_TLBP(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);
int VR4300_TLBR(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);
int VR4300_TLBWI(struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t rt);

void vr4300_cp0_init(struct vr4300 *vr4300);

#endif

