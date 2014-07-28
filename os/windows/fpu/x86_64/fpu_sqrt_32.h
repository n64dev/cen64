//
// os/windows/fpu/x86_64/fpu_sqrt_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_sqrt_32(const uint32_t *fs, uint32_t *fd) {
  float fs_float, fd_float;
  __m128 fs_reg, fd_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));

  fs_reg = _mm_load_ss(&fs_float);
  fd_reg = _mm_sqrt_ss(fs_reg);
  _mm_store_ss(&fd_float, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_float, sizeof(fd_float));
}

