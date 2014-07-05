//
// os/unix/fpu/x86_64/fpu_add_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline uint16_t fpu_add_32(
  uint32_t *fs, uint32_t *ft, uint32_t *fd) {
  uint32_t res;
  uint16_t sw;

  __asm__ volatile(
    "fclex\n\t"
    "flds %2\n\t"
    "flds %3\n\t"
    "faddp\n\t"
    "fstps %1\n\t"
    "fstsw %%ax\n\t"

    : "=a" (sw),
      "=m" (res)
    : "m" (*fs),
      "m" (*ft)
    : "st"
  );

  *fd = res;
  return sw;
}

