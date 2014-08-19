//
// arch/x86_64/rsp/rsp.h
//
// Extern declarations for host RSP functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __arch_rsp_h__
#define __arch_rsp_h__
#include "common.h"
#include <tmmintrin.h>

typedef __m128i rsp_vect_t;

// Loads and shuffles a 16x8 vector according to element.
extern const uint8_t shuffle_keys[16][16];

static inline __m128i rsp_vect_load_and_shuffle_operand(
  const uint8_t *src, unsigned element) {
  __m128i operand = _mm_load_si128((__m128i*) src);
  __m128i key = _mm_load_si128((__m128i*) shuffle_keys[element]);

  return _mm_shuffle_epi8(operand, key);
}

// Loads a vector without shuffling its elements.
static inline __m128i rsp_vect_load_unshuffled_operand(const uint8_t *src) {
  return _mm_load_si128((__m128i*) src);
}

// Writes an operand back to memory.
static inline void rsp_vect_write_operand(uint8_t *dest, __m128i src) {
  _mm_store_si128((__m128i*) dest, src);
}

#include "arch/x86_64/rsp/vand.h"
#include "arch/x86_64/rsp/vnand.h"
#include "arch/x86_64/rsp/vnor.h"
#include "arch/x86_64/rsp/vor.h"
#include "arch/x86_64/rsp/vnxor.h"
#include "arch/x86_64/rsp/vxor.h"

#endif

