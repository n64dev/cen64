//
// arch/x86_64/fpu/cvt_f32_i32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_f32_i32(const uint32_t *fs, uint32_t *fd) {
  float fd_float;
  __m128 fd_reg;

  fd_reg = _mm_setzero_ps();
  fd_reg = _mm_cvtsi32_ss(fd_reg, *fs);
  _mm_store_ss(&fd_float, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_float, sizeof(fd_float));
}

