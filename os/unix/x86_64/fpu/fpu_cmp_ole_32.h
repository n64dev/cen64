//
// os/unix/x86_64/fpu/fpu_cmp_ole_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <emmintrin.h>
#include <string.h>

static inline uint8_t fpu_cmp_ole_32(
  const uint32_t *fs, const uint32_t *ft) {
  float fs_float, ft_float;
  __m128 fs_reg, ft_reg;
  uint8_t condition;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));
  memcpy(&ft_float, ft, sizeof(ft_float));

  fs_reg = _mm_load_ss(&fs_float);
  ft_reg = _mm_load_ss(&ft_float);

  __asm__ __volatile__(
    "comiss %1, %2\n\t"
    "setae %%dl\n\t"
    "setnp %%al\n\t"
    "and %%dl, %%al\n\t"
    : "=a" (condition)
    : "x" (fs_reg),
      "x" (ft_reg)
    : "dl", "cc"
  );

  return condition;
}

