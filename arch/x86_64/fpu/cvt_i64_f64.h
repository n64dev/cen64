//
// arch/x86_64/fpu/cvt_i64_f64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_i64_f64(const uint64_t *fs, uint64_t *fd) {
  double fs_double;
  __m128d fs_reg;

  // Prevent aliasing.
  memcpy(&fs_double, fs, sizeof(fs_double));

  fs_reg = _mm_set_sd(fs_double);
  *fd = _mm_cvtsd_si64(fs_reg);
}

