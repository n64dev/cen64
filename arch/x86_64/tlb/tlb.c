//
// arch/x86_64/tlb/tlb.c: Translation lookaside buffer.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "arch/x86_64/tlb/tlb.h"
#include <emmintrin.h>

// Initializes the TLB with invalid entries.
void tlb_init(struct cen64_tlb *tlb) {
  unsigned i;

  for (i = 0; i < 32; i++)
    tlb->vpn2.data[i] = ~0;
}

// Probes the TLB for matching entry. Returns the index or -1.
unsigned tlb_probe(const struct cen64_tlb *tlb,
  uint64_t vaddr, uint8_t vasid, unsigned *index) {
  int one_hot_idx;
  uint32_t vpn2;
  unsigned i;

  vpn2 =
    (vaddr >> 35 & 0x18000000U) |
    (vaddr >> 13 & 0x7FFFFFF);

  __m128i vpn = _mm_set1_epi32(vpn2);
  __m128i asid = _mm_set1_epi8(vasid);

  // Scan 8 entries in parallel.
  for (i = 0; i < 32; i += 8) {
    __m128i check_l, check_h, vpn_check;
    __m128i check_a, check_g, asid_check;
    __m128i check;

    __m128i page_mask_l = _mm_load_si128((__m128i*) (tlb->page_mask.data + i + 0));
    __m128i page_mask_h = _mm_load_si128((__m128i*) (tlb->page_mask.data + i + 4));
    __m128i vpn_l = _mm_load_si128((__m128i*) (tlb->vpn2.data + i + 0));
    __m128i vpn_h = _mm_load_si128((__m128i*) (tlb->vpn2.data + i + 4));

    // Check for matching VPNs.
    check_l = _mm_and_si128(vpn, page_mask_l);
    check_l = _mm_cmpeq_epi32(check_l, vpn_l);
    check_h = _mm_and_si128(vpn, page_mask_h);
    check_h = _mm_cmpeq_epi32(check_h, vpn_h);
    vpn_check = _mm_packs_epi32(check_l, check_h);
    vpn_check = _mm_packs_epi16(vpn_check, vpn_check);

    // Check for matching ASID/global, too.
    check_g = _mm_loadl_epi64((__m128i*) (tlb->global + i));
    check_a = _mm_loadl_epi64((__m128i*) (tlb->asid + i));
    asid_check = _mm_cmpeq_epi8(check_a, asid);
    asid_check = _mm_or_si128(check_g, asid_check);

    // Match only on VPN match && (asid match || global)
    check = _mm_and_si128(vpn_check, asid_check);
    if ((one_hot_idx = _mm_movemask_epi8(check)) != 0) {
      *index = i + cen64_one_hot_lut[one_hot_idx & 0xFF];
      return 0;
    }
  }

  *index = 0;
  return 1;
}

// Reads data from the specified TLB index.
int tlb_read(const struct cen64_tlb *tlb, unsigned index, uint64_t *entry_hi) {
  *entry_hi =
    ((tlb->vpn2.data[index] & 0x18000000LLU) << 35) |
    ((tlb->vpn2.data[index] & 0x7FFFFFFLLU) << 13) |
    ((tlb->global[index] & 1) << 12) |
    (tlb->asid[index]);

  return 0;
}

// Writes an entry to the TLB.
int tlb_write(struct cen64_tlb *tlb, unsigned index, uint64_t entry_hi,
  uint64_t entry_lo_0, uint64_t entry_lo_1, uint32_t page_mask) {
  tlb->page_mask.data[index] = ~(page_mask >> 13);

  tlb->vpn2.data[index] =
    (entry_hi >> 35 & 0x18000000U) |
    (entry_hi >> 13 & 0x7FFFFFF);

  tlb->global[index] = (entry_lo_0 & 0x1) && (entry_lo_1 & 0x1) ? 0xFF : 0x00;
  tlb->asid[index] = entry_hi & 0xFF;
  return 0;
}

