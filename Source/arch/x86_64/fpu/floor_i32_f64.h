//
// arch/x86_64/fpu/floor_i32_f64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <smmintrin.h>
#include <string.h>

static inline void fpu_floor_i32_f64(const uint64_t *fs, uint32_t *fd) {
  double fs_double;
  __m128d fs_reg;

  // Prevent aliasing.
  memcpy(&fs_double, fs, sizeof(fs_double));

  fs_reg = _mm_set_sd(fs_double);
  fs_reg = _mm_floor_sd(fs_reg, fs_reg);
  *fd = _mm_cvtsd_si32(fs_reg);
}

