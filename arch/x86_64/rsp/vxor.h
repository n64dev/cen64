//
// arch/x86_64/rsp/vxor.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_vxor_vnxor(uint32_t iw, __m128i vs, __m128i vt) {
  __m128i vmask = _mm_load_si128((__m128i *) rsp_vlogic_mask[iw & 0x1]);

  __m128i vd = _mm_xor_si128(vs, vt);
  return _mm_xor_si128(vd, vmask);
}

