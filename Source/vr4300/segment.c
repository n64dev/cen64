//
// vr4300/segment.c: VR4300 MMU segment manager.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/segment.h"

//
// Note: sseg, ksseg, kseg0, kseg1, and kseg3 do not appear below.
//
//       As stated on pages 128, 130, and 134 of the "NEC VR43xx
//       Microprocessor User's Manual": "The VR4300 internally uses 64-bit
//       addresses. In the 32-bit mode, a 32-bit value with bits 32 through
//       63 sign-extended is used an address."
//

cen64_align(static const struct segment USEGs[], CACHE_LINE_SIZE) = {
  /* useg, suseg, kuseg. */ {
  0x0000000000000000ULL, /* start */
  0x0000000080000000ULL, /* length */
  0x0000000000000000ULL, /* offset */

  0x20,                  /* ux mask */
  true,                  /* mapped */
  true,                  /* cached */

  /* xuseg, xsuseg, xkuseg. */ }, {
  0x0000000000000000ULL, /* start */
  0x0000010000000000ULL, /* length */
  0x0000000000000000ULL, /* offset */

  0x20,                  /* ux mask */
  true,                  /* mapped */
  true,                  /* cached */
}};

/* xsseg, xksseg. */
static const struct segment XSSEG = {
  0x4000000000000000ULL, /* start */
  0x0000010000000000ULL, /* length */
  0x0000000000000000ULL, /* offset */

  0x40,                  /* sx mask */
  true,                  /* mapped */
  true,                  /* cached */
};

cen64_align(static const struct segment KSEGs[], CACHE_LINE_SIZE) = {
  /* (c)kseg0. */ {
  0xFFFFFFFF80000000ULL, /* start */
  0x0000000020000000ULL, /* length */
  0xFFFFFFFF80000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */

  /* (c)kseg1. */ }, {
  0xFFFFFFFFA0000000ULL, /* start */
  0x0000000020000000ULL, /* length */
  0xFFFFFFFFA0000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  false,                 /* cached */

  /* (c)sseg, (c)ksseg. */ }, {
  0xFFFFFFFFC0000000ULL, /* start */
  0x0000000020000000ULL, /* length */
  0x0000000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  true,                  /* mapped */
  true,                  /* cached */

  /* (c)kseg3. */ }, {
  0xFFFFFFFFE0000000ULL, /* start */
  0x0000000020000000ULL, /* length */
  0x0000000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  true,                  /* mapped */
  true,                  /* cached */
}};

static const struct segment XKSEG = {
  0xC000000000000000ULL, /* start */
  0x0000010000000000ULL, /* length */
  0x0000000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  true,                  /* mapped */
  true,                  /* cached */
};

static const struct segment XKPHYS0 = {
  0x8000000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0x8000000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */
};

static const struct segment XKPHYS1 = {
  0x8800000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0x8800000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */
};

static const struct segment XKPHYS2 = {
  0x9000000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0x9000000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  false,                 /* cached */
};

static const struct segment XKPHYS3 = {
  0x9800000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0x9800000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */
};

static const struct segment XKPHYS4 = {
  0xA000000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0xA000000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */
};

static const struct segment XKPHYS5 = {
  0xA800000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0xA800000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */
};

static const struct segment XKPHYS6 = {
  0xB000000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0xB000000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */
};

static const struct segment XKPHYS7 = {
  0xB800000000000000ULL, /* start */
  0x0000000100000000ULL, /* length */
  0xB800000000000000ULL, /* offset */

  0x80,                  /* kx mask */
  false,                 /* mapped */
  true,                  /* cached */
};

static const struct segment *kernel_segs_lut[16] = {
  &XKPHYS0,
  &XKPHYS1,
  &XKPHYS2,
  &XKPHYS3,
  &XKPHYS4,
  &XKPHYS5,
  &XKPHYS6,
  &XKPHYS7,
  &XKSEG,
  &XKSEG,
  &XKSEG,
  &XKSEG,
  &XKSEG,
  &XKSEG,
  &XKSEG,
  &XKSEG,
};


// Returns a default segment that should cause
// a cached segment miss and result in a lookup.
const struct segment* get_default_segment(void) {
  static const struct segment default_segment = {
    1ULL,
    0ULL,
    0ULL,
    0x0,
    false,
    false,
  };

  return &default_segment;
}

// Returns the segment given a CP0 status register and a virtual address.
const struct segment* get_segment(uint64_t address, uint32_t cp0_status) {
  const struct segment *seg;

  // LUT used to determine if we're in a 64-bit mode or not.
  // i.e., if we're in supervisor mode, is the ux bit set?
  cen64_align(static const uint8_t segment_mode_lut[256], CACHE_LINE_SIZE) = {
#define _ sizeof(*seg)
/*ks:sx:ux |   k   k   k   k,  s,  k,  k,  k,  u,  k,  k,  k,  ?,  k,  k,  k */
/* 0: 0: 0 |*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 0: 0: 1 |*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,_,_,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 0: 1: 0 |*/ 0,0,0,0,0,0,0,0,_,_,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 0: 1: 1 |*/ 0,0,0,0,0,0,0,0,_,_,0,0,0,0,0,0,_,_,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 1: 0: 0 |*/ _,_,_,_,_,_,_,_,0,0,_,_,_,_,_,_,0,0,_,_,_,_,_,_,0,0,_,_,_,_,_,_,
/* 1: 0: 1 |*/ _,_,_,_,_,_,_,_,0,0,_,_,_,_,_,_,_,_,_,_,_,_,_,_,0,0,_,_,_,_,_,_,
/* 1: 1: 0 |*/ _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,0,0,_,_,_,_,_,_,0,0,_,_,_,_,_,_,
/* 1: 1: 1 |*/ _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,0,0,_,_,_,_,_,_,
#undef _
  };

  unsigned segment_mode = segment_mode_lut[cp0_status & 0xFF];
  unsigned mode_and_flags_mask = cp0_status & 0x1E;

#ifndef NDEBUG
  uint64_t sexaddress = (int64_t) ((int32_t) address);

  char kernel = (cp0_status & 0x6) || ((cp0_status & 0x18) == 0);
  char supervisor = ((cp0_status & 0x6) == 0) && ((cp0_status & 0x18) == 0x8);
  char user = ((cp0_status & 0x6) == 0) && ((cp0_status & 0x18) == 0x10);

  char use_kx = kernel && (cp0_status & 0x80);
  char use_sx = supervisor && (cp0_status & 0x40);
  char use_ux = user && (cp0_status & 0x20);
  char use_64 = use_kx | use_sx | use_ux;

  // Ensure that only one of {kernel, supervisor, user} are produced.
  assert(((kernel + supervisor + user) == 1) && "Impossible situation.");

  // Ensure that either 64-bit mode is used, or the address is sign-extended.
  assert((use_64 || (sexaddress == address)) && "Invalid 32-bit address.");
#endif

  // Check for useg/suseg/kuseg or xuseg/xsuseg/xkuseg first.
  seg = (const struct segment *) ((uintptr_t) USEGs + segment_mode);

  if (address < seg->length)
    return seg;

  // If we're not in user mode...
  else if (mode_and_flags_mask != 0x10) {
    seg = &KSEGs[2];

    // Assume we're csseg and check for xsseg/xksseg.
    if ((address >> 40) == 0x400000)
      return &XSSEG;

    // If we're in kernel mode, check for ckseg0, ckseg1, and ckseg3.
    else if (mode_and_flags_mask != 0x08) {
      if (address >= KSEGs[0].start)
        return KSEGs + (address >> 29 & 0x3);

      // Check for xkseg and xkphys.
      else if ((address - XKPHYS0.start) < 0x400000FF80000000ULL)
        seg = kernel_segs_lut[address >> 59 & 0xF];
    }

    // Check matching segment or return invalid.
    if (likely((address - seg->start) < seg->length))
      return seg;
  }

  return NULL;
}

