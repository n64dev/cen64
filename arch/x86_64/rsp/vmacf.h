//
// arch/x86_64/rsp/vmulu.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

//
// TODO: CHECK ME.
//

static inline __m128i rsp_vmacf(__m128i vs, __m128i vt,
  __m128i zero, __m128i *acc_lo, __m128i *acc_md, __m128i *acc_hi) {
  __m128i lo, hi, sign, overflow_mask, sat, unsat, temp;

  lo = _mm_mullo_epi16(vs, vt);
  hi = _mm_mulhi_epi16(vs, vt);
  temp = _mm_add_epi16(lo, lo);
  sat = _mm_adds_epu16(temp, *acc_lo);
  *acc_lo = _mm_add_epi16(*acc_lo, temp);
  overflow_mask = _mm_cmpeq_epi16(sat, *acc_lo);
  overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

  sign = _mm_srli_epi16(lo, 15);
  temp = _mm_add_epi16(hi, hi);
  temp = _mm_or_si128(temp, sign);
  unsat = _mm_sub_epi16(temp, overflow_mask);
  sat = _mm_subs_epu16(temp, overflow_mask);
  overflow_mask = _mm_cmpeq_epi16(sat, unsat);
  overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

  sat = _mm_adds_epu16(unsat, *acc_md);
  *acc_md = _mm_add_epi16(*acc_md, unsat);
  temp = _mm_cmpeq_epi16(sat, *acc_md);
  temp = _mm_cmpeq_epi16(temp, zero);
  overflow_mask = _mm_sub_epi16(overflow_mask, temp);

  hi = _mm_srai_epi16(hi, 15);
  *acc_hi = _mm_add_epi16(*acc_hi, hi);
  *acc_hi = _mm_sub_epi16(*acc_hi, overflow_mask);
  return rsp_sclamp_acc_tomd(*acc_md, *acc_hi);
}
