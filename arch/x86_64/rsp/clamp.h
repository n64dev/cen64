//
// arch/x86_64/rsp/clamp.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline __m128i rsp_clamp_lo(__m128i acc_lo,
  __m128i acc_md, __m128i acc_hi, __m128i zero) {
  __m128i clamp_match_mask, sign_match_mask;
  __m128i lo, hi, md_clamped, clamp_value;

  lo = _mm_unpacklo_epi16(acc_md, acc_hi);
  hi = _mm_unpackhi_epi16(acc_md, acc_hi);
  md_clamped = _mm_packs_epi32(lo, hi);

  clamp_match_mask = _mm_cmpeq_epi16(md_clamped, acc_md);
  clamp_value = _mm_srai_epi16(md_clamped, 15);

  sign_match_mask = _mm_cmpeq_epi16(clamp_value, acc_hi);
  clamp_match_mask = _mm_and_si128(clamp_match_mask, sign_match_mask);
  clamp_value = _mm_cmpeq_epi16(clamp_value, zero);

  return _mm_blendv_epi8(clamp_value, acc_lo, clamp_match_mask);
}

