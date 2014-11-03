//
// arch/x86_64/rsp/vsub.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_vsub(__m128i vs, __m128i vt,
  __m128i zero, __m128i carry, __m128i *acc_lo) {
  __m128i vd, unsat_diff, mask, vt_neg_mask, vt_pos_mask;

  // VCC uses unsaturated arithmetic.
  unsat_diff = _mm_sub_epi16(vs, vt);
  *acc_lo = _mm_sub_epi16(unsat_diff, carry);

  // VD is the signed sum of the two sources and the carry. Since we
  // have to saturate the sum of all three, we have to be clever.
  vt_neg_mask = _mm_cmplt_epi16(vt, zero);
  vt_pos_mask = _mm_cmpeq_epi16(vt_neg_mask, zero);

  vd = _mm_subs_epi16(vs, vt);
  mask = _mm_cmpeq_epi16(unsat_diff, vd);
  mask = _mm_and_si128(vt_neg_mask, mask);

  vt_neg_mask = _mm_and_si128(mask, carry);
  vt_pos_mask = _mm_and_si128(vt_pos_mask, carry);
  carry = _mm_or_si128(vt_neg_mask, vt_pos_mask);
  return _mm_subs_epi16(vd, carry);
}

