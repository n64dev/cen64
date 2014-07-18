//
// os/unix/fpu/x86_64/fpu_cmp_olt_64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline uint16_t fpu_cmp_olt_64(
  uint64_t *fs, uint64_t *ft, uint8_t *flag) {
  uint8_t not_un, lt;
  uint16_t sw;

  __asm__ volatile(
    "fclex\n\t"
    "fldl %4\n\t"
    "fldl %3\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "setb %1\n\t"
    "fstsw %%ax\n\t"
    "fstp %%st(0)\n\t"

    : "=m" (not_un),
      "=m" (lt),
      "=a" (sw)
    : "m" (*fs),
      "m" (*ft)
    : "cc", "st"
  );

  *flag = not_un & lt;
  return sw;
}

