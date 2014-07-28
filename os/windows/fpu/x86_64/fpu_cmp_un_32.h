//
// os/windows/fpu/x86_64/fpu_cmp_un_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline void fpu_cmp_un_32(
  const uint32_t *fs, const uint32_t *ft, uint8_t *condition) {
  float fs_float, ft_float;
  __m128 fs_reg, ft_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));
  memcpy(&ft_float, ft, sizeof(ft_float));

  fs_reg = _mm_load_ss(&fs_float);
  ft_reg = _mm_load_ss(&ft_float);

  __asm__ __volatile__(
    "comiss %1, %2\n\t"
    "setp %%al\n\t"
    : "=a" (*condition)
    : "x" (fs_reg),
      "x" (ft_reg)
    : "cc"
  );
}

