//
// vr4300/cp0.c: VR4300 system control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "tlb/tlb.h"
#include "vr4300/cp0.h"
#include "vr4300/cpu.h"

static const uint64_t vr4300_cp0_reg_masks[32] = {
  0x000000008000003FULL, //  0: VR4300_CP0_REGISTER_INDEX
  0x000000000000003FULL, //  1: VR4300_CP0_REGISTER_RANDOM
  0x000000007FFFFFFFULL, //  2: VR4300_CP0_REGISTER_ENTRYLO0
  0x000000007FFFFFFFULL, //  3: VR4300_CP0_REGISTER_ENTRYLO1
  0xFFFFFFFFFFFFFFF0ULL, //  4: VR4300_CP0_REGISTER_CONTEXT
  0x0000000001FFE000ULL, //  5: VR4300_CP0_REGISTER_PAGEMASK
  0xFFFFFFFFFFFFFFFFULL, //  6: VR4300_CP0_REGISTER_WIRED
  0x0000000000000BADULL, //  7:
  0xFFFFFFFFFFFFFFFFULL, //  8: VR4300_CP0_REGISTER_BADVADDR
  0x00000000FFFFFFFFULL, //  9: VR4300_CP0_REGISTER_COUNT
  0xC00000FFFFFFE0FFULL, // 10: VR4300_CP0_REGISTER_ENTRYHI
  0x00000000FFFFFFFFULL, // 11: VR4300_CP0_REGISTER_COMPARE
  0x00000000FFFFFFFFULL, // 12: VR4300_CP0_REGISTER_STATUS
  0x00000000B000FFFFULL, // 13: VR4300_CP0_REGISTER_CAUSE
  0xFFFFFFFFFFFFFFFFULL, // 14: VR4300_CP0_REGISTER_EPC
  0x000000000000FFFFULL, // 15; VR4300_CP0_REGISTER_PRID
  0x000000007FFFFFFFULL, // 16: VR4300_CP0_REGISTER_CONFIG
  0x00000000FFFFFFFFULL, // 17: VR4300_CP0_REGISTER_LLADDR
  0x00000000FFFFFFFBULL, // 18: VR4300_CP0_REGISTER_WATCHLO
  0x000000000000000FULL, // 19: VR4300_CP0_REGISTER_WATCHHI
  0xFFFFFFFFFFFFFFFFULL, // 20: VR4300_CP0_REGISTER_XCONTEXT
  0x0000000000000BADULL, // 21:
  0x0000000000000BADULL, // 22:
  0x0000000000000BADULL, // 23:
  0x0000000000000BADULL, // 24:
  0x0000000000000BADULL, // 25:
  0x0000000000000000ULL, // 26: VR4300_CP0_REGISTER_PARITYERROR
  0x0000000000000000ULL, // 27: VR4300_CP0_REGISTER_CACHEERR
  0x000000000FFFFFC0ULL, // 28: VR4300_CP0_REGISTER_TAGLO
  0x0000000000000000ULL, // 29: VR4300_CP0_REGISTER_TAGHI
  0xFFFFFFFFFFFFFFFFULL, // 30: VR4300_CP0_REGISTER_ERROREPC
  0x0000000000000BADULL, // 31
};

static inline uint64_t mask_reg(unsigned reg, uint64_t data) {
  return vr4300_cp0_reg_masks[reg] & data;
};

//
// DMFC0
//
int VR4300_DMFC0(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RT(iw);
  unsigned src = GET_RD(iw);

  if (src == (VR4300_CP0_REGISTER_COUNT - VR4300_REGISTER_CP0_0)) {
    exdc_latch->result = (uint32_t)
      (vr4300->regs[VR4300_CP0_REGISTER_COUNT] >> 1);
  }

  else if (vr4300_cp0_reg_masks[src] == 0x0000000000000BADULL)
    exdc_latch->result = vr4300->regs[VR4300_REGISTER_CP0_0 + 7];

  else if (src + VR4300_REGISTER_CP0_0 == VR4300_CP0_REGISTER_PRID)
    exdc_latch->result = 0xb22; // Hardcoded most common revision N64

  else {
    exdc_latch->result = mask_reg(src,
      vr4300->regs[VR4300_REGISTER_CP0_0 + src]);
  }

  exdc_latch->dest = dest;
  return 0;
}

//
// DMTC0
//
int VR4300_DMTC0(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);

  switch (dest + VR4300_REGISTER_CP0_0)
  {
    case VR4300_CP0_REGISTER_COUNT:
      rt <<= 1;
      break;
    case VR4300_CP0_REGISTER_CAUSE:
      vr4300->regs[VR4300_CP0_REGISTER_CAUSE] &= ~0x0300;
      vr4300->regs[VR4300_CP0_REGISTER_CAUSE] |= rt & 0x0300;
      return 0;
    case VR4300_CP0_REGISTER_COMPARE:
      vr4300->regs[VR4300_CP0_REGISTER_CAUSE] &= ~0x8000;
      break;
    case VR4300_CP0_REGISTER_STATUS:
      icrf_latch->segment = get_segment(icrf_latch->common.pc, rt);
      exdc_latch->segment = get_default_segment();
      break;
  }

  if (vr4300_cp0_reg_masks[dest] == 0x0000000000000BADULL)
    vr4300->regs[VR4300_REGISTER_CP0_0 + 7] = rt;

  else
    vr4300->regs[VR4300_REGISTER_CP0_0 + dest] = rt;

  return 0;
}

//
// ERET
//
int VR4300_ERET(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
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
  rfex_latch->common.fault = ~0;

  vr4300->regs[PIPELINE_CYCLE_TYPE] = 4;
  pipeline->exception_history = 0;
  pipeline->fault_present = true;

  vr4300->regs[VR4300_CP0_REGISTER_STATUS] = status;
  vr4300->regs[VR4300_REGISTER_LLBIT] = 0;

  pipeline->icrf_latch.segment = get_segment(icrf_latch->pc, status);
  pipeline->exdc_latch.segment = get_default_segment();
  // vr4300->llbit = 0;
  return 1;
}

//
// MFC0
//
int VR4300_MFC0(struct vr4300 *vr4300, 
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RT(iw);
  unsigned src = GET_RD(iw);

  if (src == (VR4300_CP0_REGISTER_COUNT - VR4300_REGISTER_CP0_0)) {
    exdc_latch->result = (int32_t)
      (vr4300->regs[VR4300_CP0_REGISTER_COUNT] >> 1);
  }

  else if (vr4300_cp0_reg_masks[src] == 0x0000000000000BADULL)
    exdc_latch->result = (int32_t) vr4300->regs[VR4300_REGISTER_CP0_0 + 7];

  else if (src + VR4300_REGISTER_CP0_0 == VR4300_CP0_REGISTER_PRID)
    exdc_latch->result = 0xb22; // Hardcoded most common revision N64

  else {
    exdc_latch->result = (int32_t) mask_reg(src,
      vr4300->regs[VR4300_REGISTER_CP0_0 + src]);
  }

  exdc_latch->dest = (int32_t) dest;
  return 0;
}

//
// MTC0
//
int VR4300_MTC0(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);

  switch (dest + VR4300_REGISTER_CP0_0)
  {
    case VR4300_CP0_REGISTER_COUNT:
      rt <<= 1;
      break;
    case VR4300_CP0_REGISTER_CAUSE:
      vr4300->regs[VR4300_CP0_REGISTER_CAUSE] &= ~0x0300;
      vr4300->regs[VR4300_CP0_REGISTER_CAUSE] |= (int32_t)rt & 0x0300;
      return 0;
    case VR4300_CP0_REGISTER_COMPARE:
      vr4300->regs[VR4300_CP0_REGISTER_CAUSE] &= ~0x8000;
      break;
    case VR4300_CP0_REGISTER_STATUS:
      icrf_latch->segment = get_segment(icrf_latch->common.pc, rt);
      exdc_latch->segment = get_default_segment();
      break;
  }

  if (vr4300_cp0_reg_masks[dest] == 0x0000000000000BADULL)
    vr4300->regs[VR4300_REGISTER_CP0_0 + 7] = (int32_t) rt;

  else
    vr4300->regs[VR4300_REGISTER_CP0_0 + dest] = (int32_t) rt;

  return 0;
}

//
// TLBP
//
int VR4300_TLBP(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  uint64_t entry_hi = mask_reg(10, vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI]);
  unsigned index;

  vr4300->regs[VR4300_CP0_REGISTER_INDEX] |= 0x80000000U;

  if (!tlb_probe(&vr4300->cp0.tlb, entry_hi, entry_hi & 0xFF, &index))
    vr4300->regs[VR4300_CP0_REGISTER_INDEX] = index;

  return 0;
}

//
// TLBR
//
int VR4300_TLBR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  unsigned index = vr4300->regs[VR4300_CP0_REGISTER_INDEX] & 0x3F;
  uint64_t entry_hi;

  uint32_t page_mask = (vr4300->cp0.page_mask[index] << 1) & 0x1FFE000U;
  uint32_t pfn0 = vr4300->cp0.pfn[index][0];
  uint32_t pfn1 = vr4300->cp0.pfn[index][1];
  uint8_t state0 = vr4300->cp0.state[index][0];
  uint8_t state1 = vr4300->cp0.state[index][1];

  tlb_read(&vr4300->cp0.tlb, index, &entry_hi);
  vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI] = entry_hi;
  vr4300->regs[VR4300_CP0_REGISTER_ENTRYLO0] = (pfn0 >> 6) | state0;
  vr4300->regs[VR4300_CP0_REGISTER_ENTRYLO1] = (pfn1 >> 6) | state1;
  vr4300->regs[VR4300_CP0_REGISTER_PAGEMASK] = page_mask;
  return 0;
}

//
// TLBWI
//
int VR4300_TLBWI(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  uint64_t entry_hi = mask_reg(10, vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI]);
  uint64_t entry_lo_0 = mask_reg(2, vr4300->regs[VR4300_CP0_REGISTER_ENTRYLO0]);
  uint64_t entry_lo_1 = mask_reg(3, vr4300->regs[VR4300_CP0_REGISTER_ENTRYLO1]);
  uint32_t page_mask = mask_reg(5, vr4300->regs[VR4300_CP0_REGISTER_PAGEMASK]);
  unsigned index = vr4300->regs[VR4300_CP0_REGISTER_INDEX] & 0x3F;

  tlb_write(&vr4300->cp0.tlb, index, entry_hi, entry_lo_0, entry_lo_1, page_mask);

  vr4300->cp0.page_mask[index] = (page_mask | 0x1FFF) >> 1;
  vr4300->cp0.pfn[index][0] = (entry_lo_0 << 6) & ~0xFFFU;
  vr4300->cp0.pfn[index][1] = (entry_lo_1 << 6) & ~0xFFFU;
  vr4300->cp0.state[index][0] = entry_lo_0 & 0x3F;
  vr4300->cp0.state[index][1] = entry_lo_1 & 0x3F;
  return 0;
}

//
// TLBWR
//
int VR4300_TLBWR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  uint64_t entry_hi = mask_reg(10, vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI]);
  uint64_t entry_lo_0 = mask_reg(2, vr4300->regs[VR4300_CP0_REGISTER_ENTRYLO0]);
  uint64_t entry_lo_1 = mask_reg(3, vr4300->regs[VR4300_CP0_REGISTER_ENTRYLO1]);
  uint32_t page_mask = mask_reg(5, vr4300->regs[VR4300_CP0_REGISTER_PAGEMASK]);
  unsigned index = vr4300->regs[VR4300_CP0_REGISTER_WIRED] & 0x3F;

  index = rand() % (32 - index) + index;
  tlb_write(&vr4300->cp0.tlb, index, entry_hi, entry_lo_0, entry_lo_1, page_mask);

  vr4300->cp0.page_mask[index] = (page_mask | 0x1FFF) >> 1;
  vr4300->cp0.pfn[index][0] = (entry_lo_0 << 6) & ~0xFFFU;
  vr4300->cp0.pfn[index][1] = (entry_lo_1 << 6) & ~0xFFFU;
  vr4300->cp0.state[index][0] = entry_lo_0 & 0x3F;
  vr4300->cp0.state[index][1] = entry_lo_1 & 0x3F;
  return 0;
}

// Initializes the coprocessor.
void vr4300_cp0_init(struct vr4300 *vr4300) {
  tlb_init(&vr4300->cp0.tlb);
}
