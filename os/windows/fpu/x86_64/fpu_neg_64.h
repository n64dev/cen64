//
// os/windows/fpu/x86_64/fpu_neg_64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_neg_64(const uint64_t *fs, uint64_t *fd) {
  double fs_double, fd_double;
  __m128d fs_reg, fd_reg;

  // Prevent aliasing.
  memcpy(&fs_double, fs, sizeof(fs_double));

  fs_reg = _mm_load_sd(&fs_double);
  fd_reg = _mm_xor_pd(_mm_set_sd(-0.0), fs_reg);
  _mm_store_sd(&fd_double, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_double, sizeof(fd_double));
}

