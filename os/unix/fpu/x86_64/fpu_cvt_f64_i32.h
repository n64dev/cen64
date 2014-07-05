//
// os/unix/fpu/x86_64/fpu_cvt_f64_i32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline uint16_t fpu_cvt_f64_i32(uint32_t *fs, uint64_t *fd) {
  uint64_t res;
  uint16_t sw;

  __asm__ volatile(
    "fclex\n\t"
    "fildl %2\n\t"
    "fstpl %1\n\t"
    "fstsw %%ax\n\t"

    : "=a" (sw),
      "=m" (res)
    : "m" (*fs)
    : "st"
  );

  *fd = res;
  return sw;
}

