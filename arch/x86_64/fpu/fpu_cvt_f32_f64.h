//
// arch/x86_64/fpu/fpu_cvt_f32_f64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_f32_f64(const uint64_t *fs, uint32_t *fd) {
  double fs_double;
  float fd_float;
  __m128d fs_reg;
  __m128 fd_reg;

  // Prevent aliasing.
  memcpy(&fs_double, fs, sizeof(fs_double));
  fd_reg = _mm_setzero_ps();

  fs_reg = _mm_load_sd(&fs_double);
  fd_reg = _mm_cvtsd_ss(fd_reg, fs_reg);
  _mm_store_ss(&fd_float, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_float, sizeof(fd_float));
}

