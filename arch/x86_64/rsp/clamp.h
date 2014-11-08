//
// arch/x86_64/rsp/clamp.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline __m128i rsp_sclamp_acc_tomd(
  __m128i acc_md, __m128i acc_hi) {
  __m128i l = _mm_unpacklo_epi16(acc_md, acc_hi);
  __m128i h = _mm_unpackhi_epi16(acc_md, acc_hi);
  return _mm_packs_epi32(l, h);
}

static inline __m128i rsp_uclamp_acc(__m128i val,
  __m128i acc_md, __m128i acc_hi, __m128i zero) {
  __m128i mid_sign, neg_check, zero_check;
  __m128i clamped_val, clamp_mask;

  zero_check = _mm_cmpeq_epi16(acc_hi, zero);
  neg_check = _mm_cmplt_epi16(acc_hi, zero);
  mid_sign = _mm_srai_epi16(acc_md, 15);

  clamped_val = _mm_cmpeq_epi16(neg_check, zero);

#ifndef __SSE4_1__
  neg_check = _mm_and_si128(mid_sign, neg_check);
  zero_check = _mm_andnot_si128(mid_sign, zero_check);
  clamp_mask = _mm_or_si128(neg_check, zero_check);
#else
  clamp_mask = _mm_blendv_epi8(zero_check, neg_check, mid_sign);
#endif

#ifndef __SSE4_1__
  val = _mm_and_si128(clamp_mask, val);
  clamped_val = _mm_andnot_si128(clamp_mask, clamped_val);
  return _mm_or_si128(val, clamped_val);
#else
  return _mm_blendv_epi8(clamped_val, val, clamp_mask);
#endif
}

