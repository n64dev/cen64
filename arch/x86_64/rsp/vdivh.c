//
// arch/x86_64/rsp/vrcp.c
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "common/reciprocal.h"
#include "rsp/cpu.h"
#include "rsp/rsp.h"
#include <string.h>

__m128i rsp_vdivh(struct rsp *rsp,
  unsigned src, unsigned e, unsigned dest, unsigned de) {
  __m128i vd, vd_mask, b_result;

  int16_t elements[8];

  // Get the element from VT.
  memcpy(elements, rsp->cp2.regs + src, sizeof(elements));
  rsp->cp2.div_in = elements[e];

  // Write out the upper part of the result.
  vd_mask = _mm_load_si128((__m128i *) vdiv_mask_table[de]);
  vd = _mm_load_si128((__m128i *) (rsp->cp2.regs + dest));
  vd = _mm_andnot_si128(vd_mask, vd);

  b_result = _mm_set1_epi16(rsp->cp2.div_out);
  b_result = _mm_and_si128(vd_mask, b_result);
  return _mm_or_si128(b_result, vd);
}

