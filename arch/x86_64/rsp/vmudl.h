//
// arch/x86_64/rsp/vmudl.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

static inline __m128i rsp_vmudl(__m128i vs, __m128i vt) {
  return _mm_mulhi_epu16(vs, vt);
}

