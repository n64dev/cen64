//
// arch/x86_64/rsp/vor.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>

static inline __m128i rsp_vor(__m128i vs, __m128i vt) {
  return _mm_or_si128(vs, vt);
}

