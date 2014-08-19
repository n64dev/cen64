//
// arch/x86_64/rsp/vnxor.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>

static inline uint16x8_t rsp_vnxor(__m128i vs, __m128i vt) {
  __m128i zeroes = _mm_setzero_si128();
  __m128i ones = _mm_cmpeq_si128(zero, zero);

  return _mm_xor_si128(_mm_xor_si128(vs, vt), ones);
}

