//
// arch/x86_64/rsp/vcmp.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_veq_vge_vlt_vne(uint32_t iw, __m128i vs, __m128i vt,
  __m128i zero, __m128i *le, __m128i eq, __m128i sign) {
  __m128i equal = _mm_cmpeq_epi16(vs, vt);


  switch(iw & 3)
  {
    case 0: // VLT
    {
      __m128i lt = _mm_cmplt_epi16(vs, vt);

      equal = _mm_and_si128(eq, equal);
      equal = _mm_and_si128(sign, equal);
      *le = _mm_or_si128(lt, equal);
      break;
    }
    case 1: // VEQ
    {
      *le = _mm_andnot_si128(eq, equal);
      break;
    }
    case 2: // VNE
    {
      __m128i nequal = _mm_cmpeq_epi16(equal, zero);

      *le = _mm_or_si128(_mm_and_si128(eq, equal), nequal);
      break;
    }
    case 3: // VGE
    {
      __m128i gt = _mm_cmpgt_epi16(vs, vt);
      __m128i equalsign = _mm_and_si128(eq, sign);

      equal = _mm_andnot_si128(equalsign, equal);
      *le = _mm_or_si128(gt, equal);
      break;
    }
  }

#ifdef __SSE4_1__
  return _mm_blendv_epi8(vt, vs, *le);
#else
  vs = _mm_and_si128(*le, vs);
  vt = _mm_andnot_si128(*le, vt);
  return _mm_or_si128(vs, vt);
#endif
}

