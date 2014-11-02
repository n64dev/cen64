//
// arch/x86_64/rsp/vmudm.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_vmudm(__m128i vs, __m128i vt,
  __m128i *acc_lo, __m128i *acc_md, __m128i *acc_hi) {
  __m128i lo, hi, mask;

  lo = _mm_mullo_epi16(vs, vt);
  hi = _mm_mulhi_epu16(vs, vt);

  // What we're really want to do is signed vs * unsigned vt.
  // However, we have no such instructions to do so.
  //
  // There's a trick to "fix" an unsigned product, though:
  // If vs was negative, take the upper 16-bits of the product
  // and subtract vt.
  mask = _mm_srai_epi16(vs, 15);
  vt = _mm_and_si128(vt, mask);
  hi = _mm_sub_epi16(hi, vt);

  *acc_lo = lo;
  *acc_md = hi;
  *acc_hi = _mm_srai_epi16(hi, 15);

  return hi;
}

