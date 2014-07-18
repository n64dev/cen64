//
// os/unix/fpu/x86_64/fpu_cmp_lt_64.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline uint16_t fpu_cmp_lt_64(
  uint64_t *fs, uint64_t *ft, uint8_t *flag) {
  uint8_t un, lt;
  uint16_t sw;

  __asm__ volatile(
    "fclex\n\t"
    "fldl %4\n\t"
    "fldl %3\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "setb %1\n\t"
    "fstsw %%ax\n\t"
    "fstp %%st(0)\n\t"

    : "=m" (un),
      "=m" (lt),
      "=a" (sw)
    : "m" (*fs),
      "m" (*ft)
    : "cc", "st"
  );

  if (un)
    lt = 0;

  *flag = lt;
  return sw;
}

