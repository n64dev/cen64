//
// os/unix/fpu/x86_64/fpu_div_64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_div_64(
  const uint64_t *fs, const uint64_t *ft, uint64_t *fd) {
  double fs_double, ft_double, fd_double;
  __m128d fs_reg, ft_reg, fd_reg;

  // Prevent aliasing.
  memcpy(&fs_double, fs, sizeof(fs_double));
  memcpy(&ft_double, ft, sizeof(ft_double));

  fs_reg = _mm_load_sd(&fs_double);
  ft_reg = _mm_load_sd(&ft_double);
  fd_reg = _mm_div_sd(fs_reg, ft_reg);
  _mm_store_sd(&fd_double, fd_reg);

  // Prevent aliasing.
  memcpy(fd, &fd_double, sizeof(fd_double));
}

