//
// arch/x86_64/fpu/mul_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline void fpu_mul_32(
  const uint32_t *fs, const uint32_t *ft, uint32_t *fd) {
  float fs_float, ft_float, fd_float;
  __m128 fs_reg, ft_reg, fd_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));
  memcpy(&ft_float, ft, sizeof(ft_float));

  fs_reg = _mm_set_ss(fs_float);
  ft_reg = _mm_set_ss(ft_float);
  fd_reg = _mm_mul_ss(fs_reg, ft_reg);
  fd_float = _mm_cvtss_f32(fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_float, sizeof(fd_float));
}

