//
// os/unix/fpu/x86_64/fpu_cmp_sf_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline uint16_t fpu_cmp_sf_32(
  uint32_t *fs, uint32_t *ft, uint8_t *flag) {
  uint8_t un;
  uint16_t sw;

  __asm__ volatile(
    "fclex\n\t"
    "flds %3\n\t"
    "flds %2\n\t"
    "fcomip\n\t"
    "setp %0\n\t"
    "fstsw %%ax\n\t"
    "fstp %%st(0)\n\t"

    : "=m" (un),
      "=a" (sw)
    : "m" (*fs),
      "m" (*ft)
    : "cc", "st"
  );

  *flag = un;
  return sw;
}

