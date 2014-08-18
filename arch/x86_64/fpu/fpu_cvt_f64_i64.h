//
// arch/x86_64/fpu/fpu_cvt_f64_i64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_f64_i64(const uint64_t *fs, uint64_t *fd) {
  double fd_double;
  __m128d fd_reg;

  fd_reg = _mm_setzero_pd();
  fd_reg = _mm_cvtsi64_sd(fd_reg, *fs);
  _mm_store_sd(&fd_double, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_double, sizeof(fd_double));
}

