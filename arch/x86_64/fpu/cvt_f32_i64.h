//
// arch/x86_64/fpu/cvt_f32_i64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_f32_i64(const uint64_t *fs, uint32_t *fd) {
  float fd_float;
  __m128 fd_reg;

  fd_reg = _mm_setzero_ps();
  fd_reg = _mm_cvtsi64_ss(fd_reg, *fs);
  fd_float = _mm_cvtss_f32(fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_float, sizeof(fd_float));
}

