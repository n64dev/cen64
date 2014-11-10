//
// arch/x86_64/tlb/tlb.h: Translation lookaside buffer.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __arch_tlb_h__
#define __arch_tlb_h__
#include "common.h"

struct cen64_tlb {
  uint32_t page_mask[32];
  uint32_t vpn2[32];
  uint8_t global[32];
  uint8_t asid[32];
};

cen64_cold void tlb_init(struct cen64_tlb *tlb);

int tlb_probe(const struct cen64_tlb *tlb, uint64_t vpn2, uint8_t vasid);
int tlb_read(const struct cen64_tlb *tlb, unsigned index, uint64_t *entry_hi);
int tlb_write(struct cen64_tlb *tlb, unsigned index, uint64_t entry_hi,
  uint64_t entry_lo_0, uint64_t entry_lo_1, uint32_t page_mask);

#endif

