//
// arch/x86_64/rsp/rsp.c
//
// Declarations for host RSP functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "arch/x86_64/rsp/rsp.h"
#include "os/dynarec.h"
#include "rsp/cpu.h"

// Deallocates dynarec buffers for SSE2.
void arch_rsp_destroy(struct rsp *rsp) {
#ifndef __SSSE3__
  free_dynarec_slab(&rsp->vstore_dynarec);
#endif
}

// Allocates dynarec buffers for SSE2.
int arch_rsp_init(struct rsp *rsp) {
#ifndef __SSSE3__
  void *vload_buffer, *vstore_buffer;

  // See rsp_vload_dmem for code description.
  static const uint8_t vload_code[] = {
    0x66, 0x0F, 0x73, 0xFA, 0x00,
    0x66, 0x0F, 0x73, 0xF8, 0x00,
    0x66, 0x0F, 0x73, 0xDB, 0x00,
    0x66, 0x0F, 0xDB, 0xC3,
    0x66, 0x0F, 0x6F, 0xDA,
    0x66, 0x0F, 0x71, 0xF2, 0x08,
    0x66, 0x0F, 0x71, 0xD3, 0x08,
    0x66, 0x0F, 0xEB, 0xD3,
    0x66, 0x0F, 0x6F, 0xD8,
    0x66, 0x0F, 0x71, 0xF0, 0x08,
    0x66, 0x0F, 0x71, 0xD3, 0x08,
    0x66, 0x0F, 0xEB, 0xC3,
    0x66, 0x0F, 0xDB, 0xD0,
    0x66, 0x0F, 0xDF, 0xC1,
    0x66, 0x0F, 0xEB, 0xC2,
    0xC3
  };

  // See rsp_vstore_dmem for code description.
  static const uint8_t vstore_code[] = {
    0x66, 0x0F, 0x71, 0xF1, 0x08,
    0x66, 0x0F, 0x71, 0xD2, 0x08,
    0x66, 0x0F, 0xEB, 0xCA,
    0x66, 0x0F, 0x73, 0xF8, 0x00,
    0x66, 0x0F, 0x6F, 0xD1,
    0x66, 0x0F, 0x73, 0xF9, 0x00,
    0x66, 0x0F, 0x73, 0xDA, 0x00,
    0x66, 0x0F, 0xEB, 0xCA,
    0x66, 0x0F, 0xDB, 0xC8,
    0x66, 0x0F, 0xDF, 0xC3,
    0x66, 0x0F, 0xEB, 0xC1,
    0xC3
  };

  if ((vload_buffer = alloc_dynarec_slab(
    &rsp->vload_dynarec, CACHE_LINE_SIZE * 2)) == NULL)
    return 1;

  if ((vstore_buffer = alloc_dynarec_slab(
    &rsp->vstore_dynarec, CACHE_LINE_SIZE)) == NULL) {
    free_dynarec_slab(&rsp->vload_dynarec);
    return 1;
  }

  memcpy(vload_buffer, vload_code, sizeof(vload_code));
  memcpy(vstore_buffer, vstore_code, sizeof(vstore_code));
#endif
  return 0;
}

#ifdef __SSSE3__
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
#else
__m128i rsp_vect_load_and_shuffle_operand(
  const uint16_t *src, unsigned element) {
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

  return rsp_vect_load_unshuffled_operand(src);
}
#endif

#ifdef __SSSE3__
//
// This table also takes into account that DMEM is big-endian
// byte ordering, whereas vectors are 2-byte little-endian.
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

//
// SSSE3+ accelerated loads. Byteswap big-endian to 2-byte little-endian
// vector. Start at vector element offset, discarding any wraparound
// as necessary. Lastly, don't load across cacheline boundary.
//
// TODO: Verify wraparound behavior.
// TODO: Only tested for L{B/S/L/D/Q}V
//
__m128i rsp_vload_dmem(struct rsp *rsp,
  uint32_t addr, unsigned element, __m128i dqm, __m128i reg) {
  unsigned doffset = addr & 0xF;
  uint32_t aligned_addr = addr & 0xFF0;

  __m128i data = _mm_load_si128((__m128i *) (rsp->mem + aligned_addr));
  __m128i ekey = _mm_load_si128((__m128i *) (sll_b2l_keys[element]));
  __m128i dkey, temp;

  dqm = _mm_shuffle_epi8(dqm, ekey);

  // If the element is the bounding factor as to how much we
  // load, we'll just select the data by pushing dqm over.
  if (doffset <= element)
    dkey = _mm_load_si128((__m128i *) (sll_b2l_keys[element - doffset]));

  // If the amount of data restricts how much we'll feed
  // into the vector, we'll need to cut off the loose end
  // in addition to pushing dqm over.
  else {
    dkey = _mm_load_si128((__m128i *) (srl_b2l_keys[doffset - element]));

    temp = _mm_cmpeq_epi32(reg, reg);
    temp = _mm_shuffle_epi8(temp, dkey);
    dqm = _mm_and_si128(dqm, temp);
  }

  // Byteswap and shift as needed.
  data = _mm_shuffle_epi8(data, dkey);

  // Mask and mux in the data.
#ifdef __SSE4_1__
  data = _mm_blendv_epi8(reg, data, dqm);
#else
  data = _mm_and_si128(dqm, data);
  reg = _mm_andnot_si128(dqm, reg);
  data = _mm_or_si128(data, reg);
#endif

  return data;
}
#else
//
// SSE2 accelerated loads. Byteswap big-endian to 2-byte little-endian
// vector. Start at vector element offset, discarding any wraparound
// as necessary. Lastly, don't load across cacheline boundary.
//
// TODO: Verify wraparound behavior.
// TODO: Only tested for L{B/S/L/D/Q}V
//
__m128i rsp_vload_dmem(struct rsp *rsp,
  uint32_t addr, unsigned element, __m128i dqm, __m128i reg) {
  uint32_t aligned_addr = addr & 0xFF0;
  unsigned doffset = addr & 0xF;
  __m128i data;

  //
  // Since SSE2 only provides "fixed immediate" shuffles:
  // Patch/call a dynarec buffer that does the following:
  //
  // Given:
  //   xmm0 = DQM
  //   xmm1 = REG
  //   xmm2 = DATA
  //
  // Perform:
  //
  // 1)                          L/R shift DATA:
  //    0: 66 0F 73 FA xx        pslldq $0x0,%xmm2 ** OR **
  //    0: 66 0F 73 DA xx        psrldq $0x0,%xmm2
  //
  // 2)                          L/R shift DQM:
  //    5: 66 0F 73 F8 xx        pslldq $0x0,%xmm0
  //    A: 66 0F 73 DB xx        psrldq $0x0,%xmm3
  //    F: 66 0F DB C3           pand   %xmm3,%xmm0
  //
  // 3)                          Byteswap DATA:
  //   13: 66 0F 6F DA           movdqa %xmm2,%xmm3
  //   17: 66 0F 71 F2 08        psllw  $0x8,%xmm2
  //   1C: 66 0F 71 D3 08        psrlw  $0x8,%xmm3
  //   21: 66 0F EB D3           por    %xmm3,%xmm2
  //
  // 4)                          Byteswap DQM:
  //   25: 66 0f 6f d8           movdqa %xmm0,%xmm3
  //   29: 66 0f 71 f0 08        psllw  $0x8,%xmm0
  //   2E: 66 0f 71 d3 08        psrlw  $0x8,%xmm3
  //   33: 66 0f eb c3           por    %xmm3,%xmm0
  //
  // 5)                          Mux DATA & REG.
  //   37: 66 0F DB D0           pand   %xmm0,%xmm2
  //   3B: 66 0F DF C1           pandn  %xmm1,%xmm0
  //   3F: 66 0F EB C2           por    %xmm2,%xmm0
  //
  // 6)                          Return.
  //   43: C3                    retq
  //

  rsp->vload_dynarec.ptr[9] = element;

  // If the element is the bounding factor as to how much we
  // load, we'll just select the data by pushing dqm over.
  if (doffset <= element) {
    rsp->vload_dynarec.ptr[3] = 0xFA;
    rsp->vload_dynarec.ptr[4] = (element - doffset);
    rsp->vload_dynarec.ptr[14] = 0;
  }

  // If the amount of data restricts how much we'll feed
  // into the vector, we'll need to cut off the loose end
  // in addition to pushing dqm over.
  else {
    rsp->vload_dynarec.ptr[3] = 0xDA;
    rsp->vload_dynarec.ptr[4] = (doffset - element);
    rsp->vload_dynarec.ptr[14] = (doffset - element);
  }

  data = _mm_load_si128((__m128i *) (rsp->mem + aligned_addr));

  return ((__m128i (*)(__m128i, __m128i, __m128i, __m128i))
    rsp->vload_dynarec.ptr)(dqm, reg, data, rsp_vset());
}
#endif

#ifdef __SSSE3__
//
// These tables takes into account that vectors are 2-byte
// little-endian, whereas DMEM is big-endian byte-ordering.
//

// Rotate left LUT; rotates high order bytes back to low order.
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

//
// SSE3+ accelerated stores. Byteswap 2-byte little-endian vector back
// to big-endian. Start at vector element offset, wrapping around
// as necessary. Lastly, only store upto the cacheline boundary.
//
// TODO: Verify wraparound behavior.
// TODO: Only tested for L{B/S/L/D/Q}V
//
void rsp_vstore_dmem(struct rsp *rsp,
  uint32_t addr, unsigned element, __m128i dqm, __m128i reg) {
  unsigned doffset = addr & 0xF;
  unsigned eoffset = (doffset - element) & 0xF;
  uint32_t aligned_addr = addr & 0xFF0;

  __m128i data = _mm_load_si128((__m128i *) (rsp->mem + aligned_addr));
  __m128i dkey = _mm_load_si128((__m128i *) (sll_l2b_keys[doffset]));
  __m128i ekey = _mm_load_si128((__m128i *) (rol_l2b_keys[eoffset]));

  // Byteswap and rotate/shift using LUTs.
  reg = _mm_shuffle_epi8(reg, ekey);
  dqm = _mm_shuffle_epi8(dqm, dkey);

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

#else
//
// SSE2 accelerated stores. Byteswap 2-byte little-endian vector back
// to big-endian. Start at vector element offset, wrapping around
// as necessary. Lastly, only store upto the cacheline boundary.
//
// TODO: Verify wraparound behavior.
// TODO: Only tested for L{B/S/L/D/Q}V
//
void rsp_vstore_dmem(struct rsp *rsp,
  uint32_t addr, unsigned element, __m128i dqm, __m128i reg) {
  unsigned doffset = addr & 0xF;
  unsigned eoffset = (doffset - element) & 0xF;
  uint32_t aligned_addr = addr & 0xFF0;
  __m128i data;

  //
  // Since SSE2 only provides "fixed immediate" shuffles:
  // Patch/call a dynarec buffer that does the following:
  //
  // Given:
  //   xmm0 = DQM
  //   xmm1 = REG
  //   xmm2 = REG
  //   xmm3 = DATA
  //
  // Perform:
  //
  // 1)                          Byteswap REG.
  //    0: 66 0F 71 F1 08        psllw  $0x8,%xmm1
  //    5: 66 0F 71 D2 08        psrlw  $0x8,%xmm2
  //    A: 66 0F EB CA           por    %xmm2,%xmm1
  //
  // 2)                          Shift DQM left.
  //    E: 66 0F 73 F8 00        pslldq $0x0,%xmm0
  //
  // 3)                          Rotate REG left.
  //   13: 66 0F 6F D1           movdqa %xmm1,%xmm2
  //   17: 66 0F 73 F9 xx        pslldq $0x0,%xmm1
  //   1C: 66 0F 73 DA xx        psrldq $0x0,%xmm2
  //   21: 66 0F EB CA           por    %xmm2,%xmm1
  //
  // 4)                          Mux DATA & REG.
  //   25: 66 0F DB C8           pand   %xmm0,%xmm1
  //   29: 66 0F DF C3           pandn  %xmm3,%xmm0
  //   2D: 66 0F EB C1           por    %xmm1,%xmm0
  //
  // 5)                          Return.
  //   2E: C3                    retq
  //
  rsp->vstore_dynarec.ptr[18] = doffset;
  rsp->vstore_dynarec.ptr[27] = eoffset;
  rsp->vstore_dynarec.ptr[32] = 16 - eoffset;

  data = _mm_load_si128((__m128i *) (rsp->mem + aligned_addr));

  data = ((__m128i (*)(__m128i, __m128i, __m128i, __m128i))
    rsp->vstore_dynarec.ptr)(dqm, reg, reg, data);

  _mm_store_si128((__m128i *) (rsp->mem + aligned_addr), data);
}
#endif

