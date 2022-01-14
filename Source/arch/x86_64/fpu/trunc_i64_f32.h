//
// arch/x86_64/fpu/trunc_i64_f32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_trunc_i64_f32(const uint32_t *fs, uint64_t *fd) {
  float fs_float;
  __m128 fs_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));

  fs_reg = _mm_set_ss(fs_float);
  *fd = _mm_cvttss_si64(fs_reg);
}

