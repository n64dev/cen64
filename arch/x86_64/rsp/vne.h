//
// arch/x86_64/rsp/vne.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_vne(__m128i vs, __m128i vt,
  __m128i zero, __m128i *le, __m128i eq) {
  __m128i equal = _mm_cmpeq_epi16(vs, vt);
  __m128i nequal = _mm_cmpeq_epi16(equal, zero);

  *le = _mm_and_si128(eq, equal);
  *le = _mm_or_si128(*le, nequal);

#ifdef __SSE4_1__
  return _mm_blendv_epi8(vt, vs, *le);
#else
  vs = _mm_and_si128(*le, vs);
  vt = _mm_andnot_si128(*le, vt);
  return _mm_or_si128(vs, vt);
#endif
}

