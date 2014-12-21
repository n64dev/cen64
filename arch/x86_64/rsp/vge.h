//
// arch/x86_64/rsp/vge.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_vge(__m128i vs, __m128i vt,
  __m128i *le, __m128i eq, __m128i sign) {
  __m128i equalsign = _mm_and_si128(eq, sign);
  __m128i equal = _mm_cmpeq_epi16(vs, vt);
  __m128i gt = _mm_cmpgt_epi16(vs, vt);

  equal = _mm_andnot_si128(equalsign, equal);
  *le = _mm_or_si128(gt, equal);

#ifdef __SSE4_1__
  return _mm_blendv_epi8(vt, vs, *le);
#else
  vs = _mm_and_si128(*le, vs);
  vt = _mm_andnot_si128(*le, vt);
  return _mm_or_si128(vs, vt);
#endif
}

