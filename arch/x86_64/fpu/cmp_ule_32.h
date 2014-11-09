//
// arch/x86_64/fpu/cmp_ule_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include <emmintrin.h>
#include <string.h>

static inline uint8_t fpu_cmp_ule_32(
  const uint32_t *fs, const uint32_t *ft) {
  float fs_float, ft_float;
  __m128 fs_reg, ft_reg;

  // Prevent aliasing.
  memcpy(&fs_float, fs, sizeof(fs_float));
  memcpy(&ft_float, ft, sizeof(ft_float));

  fs_reg = _mm_set_ss(fs_float);
  ft_reg = _mm_set_ss(ft_float);
  return _mm_comile_ss(fs_reg, ft_reg);
}

