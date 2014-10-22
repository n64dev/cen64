//
// arch/x86_64/rsp/vnand.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_vnand(__m128i vs, __m128i vt, __m128i zero) {
  __m128i ones = _mm_cmpeq_epi32(zero, zero);

  return _mm_xor_si128(_mm_and_si128(vs, vt), ones);
}

