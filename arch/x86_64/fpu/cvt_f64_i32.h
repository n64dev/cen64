//
// arch/x86_64/fpu/cvt_f64_i32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_f64_i32(const uint32_t *fs, uint64_t *fd) {
  double fd_double;
  __m128d fd_reg;

  fd_reg = _mm_setzero_pd();
  fd_reg = _mm_cvtsi32_sd(fd_reg, *fs);
  fd_double = _mm_cvtsd_f64(fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_double, sizeof(fd_double));
}

