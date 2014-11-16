//
// arch/x86_64/fpu/ceil_i64_f32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <smmintrin.h>
#include <string.h>

static inline void fpu_ceil_i64_f32(const uint32_t *fs, uint64_t *fd) {
  float fs_float;
  __m128 fs_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));

  fs_reg = _mm_set_ss(fs_float);
  fs_reg = _mm_ceil_ss(fs_reg, fs_reg);
  *fd = _mm_cvtss_si64(fs_reg);
}

