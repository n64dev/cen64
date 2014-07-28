//
// os/windows/fpu/x86_64/fpu_cmp_f_64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_cmp_f_64(
  const uint64_t *fs, const uint64_t *ft, uint8_t *condition) {
  double fs_double, ft_double;
  __m128d fs_reg, ft_reg;

  // Prevent aliasing.
  memcpy(&fs_double, fs, sizeof(fs_double));
  memcpy(&ft_double, ft, sizeof(ft_double));

  fs_reg = _mm_load_sd(&fs_double);
  ft_reg = _mm_load_sd(&ft_double);
  *condition = 0;

  __asm__ __volatile__(
    "comisd %0, %1\n\t"
    :: "x" (fs_reg),
       "x" (ft_reg)
    : "cc"
  );
}

