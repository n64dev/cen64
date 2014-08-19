//
// arch/x86_64/rsp/vand.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>

static inline uint16x8_t rsp_vand(__m128i vs, __m128i vt) {
  return _mm_and_si128(vs, vt);
}

