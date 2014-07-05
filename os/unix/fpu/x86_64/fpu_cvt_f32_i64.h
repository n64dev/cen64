//
// os/unix/fpu/x86_64/fpu_cvt_f32_i64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline uint16_t fpu_cvt_f32_i64(uint64_t *fs, uint32_t *fd) {
  uint32_t res;
  uint16_t sw;

  __asm__ volatile(
    "fclex\n\t"
    "fildq %2\n\t"
    "fstps %1\n\t"
    "fstsw %%ax\n\t"

    : "=a" (sw),
      "=m" (res)
    : "m" (*fs)
    : "st"
  );

  *fd = res;
  return sw;
}

