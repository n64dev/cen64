//
// arch/x86_64/fpu/neg_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_neg_32(const uint32_t *fs, uint32_t *fd) {
  float fs_float, fd_float;
  __m128 fs_reg, fd_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));

  fs_reg = _mm_set_ss(fs_float);
  fd_reg = _mm_xor_ps(_mm_set_ss(-0.0f), fs_reg);
  _mm_store_ss(&fd_float, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_float, sizeof(fd_float));
}

