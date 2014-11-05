//
// arch/x86_64/rsp/clamp.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline __m128i rsp_clamp_lo(__m128i acc_lo,
  __m128i acc_md, __m128i acc_hi, __m128i zero) {
  __m128i unclamped_val, mid_sign, neg_check, zero_check;
  __m128i clamped_val, clamp_mask;

  zero_check = _mm_cmpeq_epi16(acc_hi, zero);
  neg_check = _mm_cmplt_epi16(acc_hi, zero);
  mid_sign = _mm_srai_epi16(acc_md, 15);
  unclamped_val = acc_lo;

  clamped_val = _mm_cmpeq_epi16(neg_check, zero);

#ifndef __SSE4_1__
  neg_check = _mm_and_si128(mid_sign, neg_check);
  zero_check = _mm_andnot_si128(mid_sign, zero_check);
  clamp_mask = _mm_or_si128(neg_check, zero_check);
#else
  clamp_mask = _mm_blendv_epi8(zero_check, neg_check, mid_sign);
#endif

#ifndef __SSE4_1__
  unclamped_val = _mm_and_si128(clamp_mask, unclamped_val);
  clamped_val = _mm_andnot_si128(clamp_mask, clamped_val);
  return _mm_or_si128(unclamped_val, clamped_val);
#else
  return _mm_blendv_epi8(clamped_val, unclamped_val, clamp_mask);
#endif
}

