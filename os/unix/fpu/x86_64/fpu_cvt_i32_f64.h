//
// os/unix/fpu/x86_64/fpu_cvt_i32_f64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_cvt_i32_f64(const uint64_t *fs, uint32_t *fd) {
  double fs_double;
  __m128d fs_reg;

  // Prevent aliasing.
  memcpy(&fs_double, fs, sizeof(fs_double));

  fs_reg = _mm_load_sd(&fs_double);
  *fd = _mm_cvtsd_si32(fs_reg);
}

