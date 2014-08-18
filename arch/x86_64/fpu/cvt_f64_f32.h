//
// arch/x86_64/fpu/cvt_f64_f32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_f64_f32(const uint32_t *fs, uint64_t *fd) {
  double fd_double;
  float fs_float;
  __m128d fd_reg;
  __m128 fs_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));
  fd_reg = _mm_setzero_pd();

  fs_reg = _mm_load_ss(&fs_float);
  fd_reg = _mm_cvtss_sd(fd_reg, fs_reg);
  _mm_store_sd(&fd_double, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_double, sizeof(fd_double));
}

