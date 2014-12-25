//
// arch/x86_64/rsp/rsp.c
//
// Declarations for host RSP functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/dynarec.h"
#include "rsp/cpu.h"
#include "rsp/pipeline.h"
#include "rsp/rsp.h"

#ifdef __SSSE3__
//
// This table is used to "shuffle" the RSP vector after loading it.
//
cen64_align(const uint16_t shuffle_keys[16][8], CACHE_LINE_SIZE)  = {
  /* -- */ {0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0B0A, 0x0D0C, 0x0F0E},
  /* -- */ {0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0B0A, 0x0D0C, 0x0F0E},

  /* 0q */ {0x0100, 0x0100, 0x0504, 0x0504, 0x0908, 0x0908, 0x0D0C, 0x0D0C},
  /* 1q */ {0x0302, 0x0302, 0x0706, 0x0706, 0x0B0A, 0x0B0A, 0x0F0E, 0x0F0E},

  /* 0h */ {0x0100, 0x0100, 0x0100, 0x0100, 0x0908, 0x0908, 0x0908, 0x0908},
  /* 1h */ {0x0302, 0x0302, 0x0302, 0x0302, 0x0B0A, 0x0B0A, 0x0B0A, 0x0B0A},
  /* 2h */ {0x0504, 0x0504, 0x0504, 0x0504, 0x0D0C, 0x0D0C, 0x0D0C, 0x0D0C},
  /* 3h */ {0x0706, 0x0706, 0x0706, 0x0706, 0x0F0E, 0x0F0E, 0x0F0E, 0x0F0E},

  /* 0w */ {0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100},
  /* 1w */ {0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302},
  /* 2w */ {0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504},
  /* 3w */ {0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706},
  /* 4w */ {0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908},
  /* 5w */ {0x0B0A, 0x0B0A, 0x0B0A, 0x0B0A, 0x0B0A, 0x0B0A, 0x0B0A, 0x0B0A},
  /* 6w */ {0x0D0C, 0x0D0C, 0x0D0C, 0x0D0C, 0x0D0C, 0x0D0C, 0x0D0C, 0x0D0C},
  /* 7w */ {0x0F0E, 0x0F0E, 0x0F0E, 0x0F0E, 0x0F0E, 0x0F0E, 0x0F0E, 0x0F0E},
};

//
// These tables are used to shift data loaded from DMEM.
// In addition to shifting, they also take into account that
// DMEM uses big-endian byte ordering, whereas vectors are
// 2-byte little-endian.
//

// Shift left LUT; shifts in zeros from the right, one byte at a time.
cen64_align(const uint16_t sll_b2l_keys[16][8], CACHE_LINE_SIZE) = {
  {0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F},
  {0x8000, 0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E},
  {0x8080, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D},
  {0x8080, 0x8000, 0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C},

  {0x8080, 0x8080, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B},
  {0x8080, 0x8080, 0x8000, 0x0102, 0x0304, 0x0506, 0x0708, 0x090A},
  {0x8080, 0x8080, 0x8080, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809},
  {0x8080, 0x8080, 0x8080, 0x8000, 0x0102, 0x0304, 0x0506, 0x0708},

  {0x8080, 0x8080, 0x8080, 0x8080, 0x0001, 0x0203, 0x0405, 0x0607},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8000, 0x0102, 0x0304, 0x0506},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0001, 0x0203, 0x0405},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8000, 0x0102, 0x0304},

  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0001, 0x0203},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8000, 0x0102},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0001},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8000},
};

// Shift left LUT; shirts low order to high order, inserting 0x00s.
cen64_align(const uint16_t sll_l2b_keys[16][8], CACHE_LINE_SIZE) = {
  {0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F},
  {0x0180, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0E0C},
  {0x8080, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D},
  {0x8080, 0x0180, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A},

  {0x8080, 0x8080, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B},
  {0x8080, 0x8080, 0x0180, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08},
  {0x8080, 0x8080, 0x8080, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809},
  {0x8080, 0x8080, 0x8080, 0x0180, 0x0300, 0x0502, 0x0704, 0x0906},

  {0x8080, 0x8080, 0x8080, 0x8080, 0x0001, 0x0203, 0x0405, 0x0607},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x0180, 0x0300, 0x0502, 0x0704},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0001, 0x0203, 0x0405},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0180, 0x0300, 0x0502},

  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0001, 0x0203},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0180, 0x0300},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0001},
  {0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0180},
};

// Shift right LUT; shifts in zeros from the left, one byte at a time.
cen64_align(const uint16_t srl_b2l_keys[16][8], CACHE_LINE_SIZE) = {
  {0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F},
  {0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F80},
  {0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x8080},
  {0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F80, 0x8080},

  {0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x8080, 0x8080},
  {0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F80, 0x8080, 0x8080},
  {0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x8080, 0x8080, 0x8080},
  {0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F80, 0x8080, 0x8080, 0x8080},

  {0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x8080, 0x8080, 0x8080, 0x8080},
  {0x090A, 0x0B0C, 0x0D0E, 0x0F80, 0x8080, 0x8080, 0x8080, 0x8080},
  {0x0A0B, 0x0C0D, 0x0E0F, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080},
  {0x0B0C, 0x0D0E, 0x0F80, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080},

  {0x0C0D, 0x0E0F, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080},
  {0x0D0E, 0x0F80, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080},
  {0x0E0F, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080},
  {0x0F80, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080},
};

cen64_align(const uint16_t rol_b2l_keys[16][8], CACHE_LINE_SIZE) = {
  {0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F},
  {0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F00},
  {0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001},
  {0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F00, 0x0102},

  {0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203},
  {0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F00, 0x0102, 0x0304},
  {0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405},
  {0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F00, 0x0102, 0x0304, 0x0506},

  {0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607},
  {0x090A, 0x0B0C, 0x0D0E, 0x0F00, 0x0102, 0x0304, 0x0506, 0x0708},
  {0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809},
  {0x0B0C, 0x0D0E, 0x0F00, 0x0102, 0x0304, 0x0506, 0x0708, 0x090A},

  {0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B},
  {0x0D0E, 0x0F00, 0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C},
  {0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D},
  {0x0F00, 0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E},
};

// Rotate left LUT; rotates high order bytes back to low order.
#if 1
cen64_align(const uint16_t rol_l2b_keys[16][8], CACHE_LINE_SIZE) = {
  {0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F},
  {0x010E, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C},
  {0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D},
  {0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A},

  {0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B},
  {0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08},
  {0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809},
  {0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704, 0x0906},

  {0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607},
  {0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704},
  {0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405},
  {0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502},

  {0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203},
  {0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300},
  {0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001},
  {0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E},
};
#endif

// Rotate right LUT; rotates high order bytes back to low order.
cen64_align(const uint16_t ror_l2b_keys[16][8], CACHE_LINE_SIZE) = {
  {0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F},
  {0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E},
  {0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001},
  {0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300},

  {0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203},
  {0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502},
  {0x0607, 0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405},
  {0x0906, 0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704},

  {0x0809, 0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607},
  {0x0B08, 0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704, 0x0906},
  {0x0A0B, 0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809},
  {0x0D0A, 0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08},

  {0x0C0D, 0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B},
  {0x0F0C, 0x010E, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A},
  {0x0E0F, 0x0001, 0x0203, 0x0405, 0x0607, 0x0809, 0x0A0B, 0x0C0D},
  {0x010E, 0x0300, 0x0502, 0x0704, 0x0906, 0x0B08, 0x0D0A, 0x0F0C},
};
#endif

// Deallocates dynarec buffers for SSE2.
void arch_rsp_destroy(struct rsp *rsp) {}

// Allocates dynarec buffers for SSE2.
int arch_rsp_init(struct rsp *rsp) { return 0; }

#ifndef __SSSE3__
__m128i rsp_vect_load_and_shuffle_operand(
  const uint16_t *srcp, unsigned element) {
  uint16_t word_lo, word_hi;
  uint64_t dword;

  // element => 0w ... 7w
  if (element >= 8) {
    memcpy(&word_lo, src + (element - 8), sizeof(word_lo));
    dword = word_lo | ((uint32_t) word_lo << 16);

    return _mm_shuffle_epi32(_mm_loadl_epi64((__m128i *) &dword),
      _MM_SHUFFLE(0,0,0,0));
  }

  // element => 0h ... 3h
  else if (element >= 4) {
    __m128i v;

    memcpy(&word_hi, src + element - 0, sizeof(word_hi));
    memcpy(&word_lo, src + element - 4, sizeof(word_lo));
    dword = word_lo | ((uint32_t) word_hi << 16);

    v = _mm_loadl_epi64((__m128i *) &dword);
    v = _mm_shufflelo_epi16(v, _MM_SHUFFLE(1,1,0,0));
    return _mm_shuffle_epi32(v, _MM_SHUFFLE(1,1,0,0));
  }

  // element => 0q ... 1q
  else if (element >= 2) {
    __m128i vlo, vhi;
    int i;

    dword = src[element - 2];

    for (i = -1; i < 6; i += 2)
      dword = (dword << 16) | src[element + i];

    vlo = _mm_loadl_epi64((__m128i *) &dword);
    vhi = _mm_slli_si128(vlo, 8);
    vlo = _mm_shufflelo_epi16(vlo, _MM_SHUFFLE(2,2,3,3));
    vhi = _mm_shufflehi_epi16(vhi, _MM_SHUFFLE(0,0,1,1));
    return _mm_or_si128(vhi, vlo);
  }

  return rsp_vect_load_unshuffled_operand((__m128i *) src);
}
#endif

#ifdef __SSSE3__
//
// SSSE3+ accelerated loads for group I. Byteswap big-endian to 2-byte
// little-endian vector. Start at vector element offset, discarding any
// wraparound as necessary.
//
// TODO: Reverse-engineer what happens when loads to vector elements must
//       wraparound. Do we just discard the data, as below, or does the
//       data effectively get rotated around the edge of the vector?
//
void rsp_vload_group1(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm) {
  __m128i ekey, data;

  unsigned offset = addr & 0x7;

  // Always load in 8-byte chunks to emulate wraparound.
  if (offset) {
    uint32_t aligned_addr_lo = addr & ~0x7;
    uint32_t aligned_addr_hi = (aligned_addr_lo + 8) & 0xFFF;
    __m128i temp;

    data = _mm_loadl_epi64((__m128i *) (rsp->mem + aligned_addr_lo));
    temp = _mm_loadl_epi64((__m128i *) (rsp->mem + aligned_addr_hi));
    data = _mm_unpacklo_epi64(data, temp);
  }

  else
    data = _mm_loadl_epi64((__m128i *) (rsp->mem + addr));

  // Shift the DQM up to the point where we mux in the data.
  ekey = _mm_load_si128((__m128i *) (sll_b2l_keys + element));
  dqm = _mm_shuffle_epi8(dqm, ekey);

  // Align the data to the DQM so we can mask it in.
  ekey = (element > offset)
    ? _mm_load_si128((__m128i *) (sll_b2l_keys + (element - offset)))
    : _mm_load_si128((__m128i *) (srl_b2l_keys + (offset - element)));

  data = _mm_shuffle_epi8(data, ekey);

  // Mask and mux in the data.
#ifdef __SSE4_1__
  data = _mm_blendv_epi8(reg, data, dqm);
#else
  data = _mm_and_si128(dqm, data);
  reg = _mm_andnot_si128(dqm, reg);
  data = _mm_or_si128(data, reg);
#endif

  _mm_store_si128((__m128i *) regp, data);
}

//
// SSSE3+ accelerated loads for group II.
//
// TODO: Reverse-engineer what happens when loads to vector elements must
//       wraparound. Do we just discard the data, as below, or does the
//       data effectively get rotated around the edge of the vector?
//
// TODO: Reverse-engineer what happens when element != 0.
//
void rsp_vload_group2(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm) {
  unsigned offset = addr & 0x7;
  __m128i data, zero;

  // Always load in 8-byte chunks to emulate wraparound.
  if (offset) {
    uint32_t aligned_addr_lo = addr & ~0x7;
    uint32_t aligned_addr_hi = (aligned_addr_lo + 8) & 0xFFF;
    uint64_t datalow, datahigh;

    memcpy(&datalow, rsp->mem + aligned_addr_lo, sizeof(datalow));
    memcpy(&datahigh, rsp->mem + aligned_addr_hi, sizeof(datahigh));

    // TODO: Test for endian issues?
    datahigh >>= ((8 - offset) << 3);
    datalow <<= (offset << 3);
    datalow = datahigh | datalow;

    data = _mm_loadl_epi64((__m128i *) &datalow);
  }

  else
    data = _mm_loadl_epi64((__m128i *) (rsp->mem + addr));

  // "Unpack" the data.
  zero = _mm_setzero_si128();
  data = _mm_unpacklo_epi8(zero, data);

  if (rsp->pipeline.exdf_latch.request.type != RSP_MEM_REQUEST_PACK)
    data = _mm_srli_epi16(data, 1);

  _mm_store_si128((__m128i *) regp, data);
}

//
// SSSE3+ accelerated loads for group IV. Byteswap big-endian to 2-byte
// little-endian vector. Stop loading at quadword boundaries.
//
// TODO: Reverse-engineer what happens when loads from vector elements
//       must wraparound (i.e., the address offset is small, starting
//       element is large).
//
void rsp_vload_group4(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm) {
  uint32_t aligned_addr = addr & 0xFF0;
  unsigned offset = addr & 0xF;
  unsigned rol;

  __m128i data = _mm_load_si128((__m128i *) (rsp->mem + aligned_addr));
  __m128i dkey;

  if (rsp->pipeline.exdf_latch.request.type == RSP_MEM_REQUEST_QUAD)
    rol = 16 - element + offset;

  // TODO: How is this adjusted for LRV when e != 0?
  else {
    dqm = _mm_cmpeq_epi8(_mm_setzero_si128(), dqm);
    rol = 16 - offset;
  }

  dkey = _mm_load_si128((__m128i *) (rol_b2l_keys[rol & 0xF]));
  data = _mm_shuffle_epi8(data, dkey);
  dqm = _mm_shuffle_epi8(dqm, dkey);

  // Mask and mux in the data.
#ifdef __SSE4_1__
  data = _mm_blendv_epi8(reg, data, dqm);
#else
  data = _mm_and_si128(dqm, data);
  reg = _mm_andnot_si128(dqm, reg);
  data = _mm_or_si128(data, reg);
#endif

  _mm_store_si128((__m128i *) regp, data);
}

//
// SSE3+ accelerated stores for group I. Byteswap 2-byte little-endian
// vector back to big-endian. Start at vector element offset, wrapping
// around the edge of the vector as necessary.
//
// TODO: Reverse-engineer what happens when stores from vector elements
//       must wraparound. Do we just stop storing the data, or do we
//       continue storing from the front of the vector, as below?
//
void rsp_vstore_group1(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm) {
  unsigned offset = addr & 0x7;
  __m128i ekey, data;

  // Shift the DQM up to the point where we mux in the data.
  ekey = _mm_load_si128((__m128i *) (sll_l2b_keys + offset));
  dqm = _mm_shuffle_epi8(dqm, ekey);

  // Rotate the reg to align with the DQM.
  ekey = (element > offset)
    ? _mm_load_si128((__m128i *) (ror_l2b_keys + (element - offset)))
    : _mm_load_si128((__m128i *) (sll_l2b_keys + (offset - element)));

  reg = _mm_shuffle_epi8(reg, ekey);

  // Always load in 8-byte chunks to emulate wraparound.
  if (offset) {
    uint32_t aligned_addr_lo = addr & ~0x7;
    uint32_t aligned_addr_hi = (aligned_addr_lo + 8) & 0xFFF;
    __m128i temp;

    data = _mm_loadl_epi64((__m128i *) (rsp->mem + aligned_addr_lo));
    temp = _mm_loadl_epi64((__m128i *) (rsp->mem + aligned_addr_hi));
    data = _mm_unpacklo_epi64(data, temp);

  // Mask and mux in the data.
#ifdef __SSE4_1__
  data = _mm_blendv_epi8(data, reg, dqm);
#else
  data = _mm_andnot_si128(dqm, data);
  reg = _mm_and_si128(dqm, reg);
  data = _mm_or_si128(data, reg);
#endif

    _mm_storel_epi64((__m128i *) (rsp->mem + aligned_addr_lo), data);

    data = _mm_srli_si128(data, 8);
    _mm_storel_epi64((__m128i *) (rsp->mem + aligned_addr_hi), data);
  }

  else {
    data = _mm_loadl_epi64((__m128i *) (rsp->mem + addr));

  // Mask and mux in the data.
#ifdef __SSE4_1__
  data = _mm_blendv_epi8(data, reg, dqm);
#else
  data = _mm_andnot_si128(dqm, data);
  reg = _mm_and_si128(dqm, reg);
  data = _mm_or_si128(data, reg);
#endif

    _mm_storel_epi64((__m128i *) (rsp->mem + addr), data);
  }
}

//
// SSE3+ accelerated stores for group II. Byteswap 2-byte little-endian
// vector back to big-endian. Start at vector element offset, wrapping
// around the edge of the vector as necessary.
//
// TODO: Reverse-engineer what happens when stores from vector elements
//       must wraparound. Do we just stop storing the data, or do we
//       continue storing from the front of the vector, as below?
//
// TODO: Reverse-engineer what happens when element != 0.
//
void rsp_vstore_group2(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm) {

  // "Pack" the data.
  if (rsp->pipeline.exdf_latch.request.type != RSP_MEM_REQUEST_PACK)
    reg = _mm_slli_epi16(reg, 1);

  reg = _mm_srai_epi16(reg, 8);
  reg = _mm_packs_epi16(reg, reg);

  // TODO: Always store in 8-byte chunks to emulate wraparound.
  _mm_storel_epi64((__m128i *) (rsp->mem + addr), reg);
}

//
// SSE3+ accelerated stores for group IV. Byteswap 2-byte little-endian
// vector back to big-endian. Stop storing at quadword boundaries.
//
void rsp_vstore_group4(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm) {
  uint32_t aligned_addr = addr & 0xFF0; 
  unsigned offset = addr & 0xF;
  unsigned rol = offset;

  __m128i data = _mm_load_si128((__m128i *) (rsp->mem + aligned_addr));
  __m128i ekey;

  if (rsp->pipeline.exdf_latch.request.type == RSP_MEM_REQUEST_QUAD)
    rol -= element;

  // TODO: How is this adjusted for SRV when e != 0?
  else
    dqm = _mm_cmpeq_epi8(_mm_setzero_si128(), dqm);

  ekey = _mm_load_si128((__m128i *) (rol_l2b_keys[rol & 0xF]));
  reg = _mm_shuffle_epi8(reg, ekey);

  // Mask and mux out the data, write.
#ifdef __SSE4_1__
  data = _mm_blendv_epi8(data, reg, dqm);
#else
  reg = _mm_and_si128(dqm, reg);
  data = _mm_andnot_si128(dqm, data);
  data = _mm_or_si128(data, reg);
#endif

  _mm_store_si128((__m128i *) (rsp->mem + aligned_addr), data);
}
#endif

