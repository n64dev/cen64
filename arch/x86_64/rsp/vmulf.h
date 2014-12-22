//
// arch/x86_64/rsp/vmulf.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline __m128i rsp_vmulf(__m128i vs, __m128i vt, __m128i zero,
  __m128i *acc_lo, __m128i *acc_md, __m128i *acc_hi) {
 __m128i lo, hi, round, sign1, sign2, neq, eq, neg;

  lo = _mm_mullo_epi16(vs, vt);
  round = _mm_cmpeq_epi16(zero, zero);
  sign1 = _mm_srli_epi16(lo, 15);
  lo = _mm_add_epi16(lo, lo);
  round = _mm_slli_epi16(round, 15);
  hi = _mm_mulhi_epi16(vs, vt);
  sign2 = _mm_srli_epi16(lo, 15);
  *acc_lo = _mm_add_epi16(round, lo);
  sign1 = _mm_add_epi16(sign1, sign2);

  hi = _mm_slli_epi16(hi, 1);
  neq = eq = _mm_cmpeq_epi16(vs, vt);
  *acc_md = _mm_add_epi16(hi, sign1);

  neg = _mm_srai_epi16(*acc_md, 15);
  eq = _mm_and_si128(eq, neg);
  *acc_hi = _mm_andnot_si128(neq, neg);
  return _mm_add_epi16(*acc_md, eq);
}

