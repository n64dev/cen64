//
// arch/x86_64/rsp/clamp.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline __m128i rsp_clamp_lo(__m128i acc_lo,
  __m128i acc_md, __m128i acc_hi, __m128i zero) {
  __m128i mid_sign, neg_check, neg_val, pos_val, use_val_mask;

  neg_check = _mm_cmplt_epi16(acc_hi, zero);
  mid_sign = _mm_srai_epi16(acc_md, 15);

  neg_val = _mm_and_si128(neg_check, acc_hi);
  use_val_mask = _mm_and_si128(mid_sign, neg_val);
  neg_val = _mm_and_si128(use_val_mask, acc_lo);

  use_val_mask = _mm_or_si128(acc_hi, mid_sign);
  use_val_mask = _mm_cmpeq_epi16(use_val_mask, zero);
  pos_val = _mm_and_si128(use_val_mask, acc_lo);

#ifndef __SSE4_1__
  neg_val = _mm_and_si128(neg_check, neg_val);
  pos_val = _mm_andnot_si128(neg_check, pos_val);
  return _mm_or_si128(neg_val, pos_val);
#else
  return _mm_blendv_epi8(pos_val, neg_val, neg_check);
#endif
}

