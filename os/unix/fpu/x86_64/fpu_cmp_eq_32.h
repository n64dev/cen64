//
// os/unix/fpu/x86_64/fpu_cmp_eq_32.h
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

static inline uint16_t fpu_cmp_eq_32(
  uint32_t *fs, uint32_t *ft, uint8_t *flag) {
  uint8_t not_un, eq;
  uint16_t sw;

  // XXX: Check; blindly copied from old core.
  __asm__ volatile(
    "fclex\n\t"
    "flds %4\n\t"
    "flds %3\n\t"
    "fcomip\n\t"
    "setnp %0\n\t"
    "sete %1\n\t"
    "fstsw %%ax\n\t"
    "fstp %%st(0)\n\t"

    : "=m" (not_un),
      "=m" (eq),
      "=a" (sw)
    : "m" (*fs),
      "m" (*ft)
    : "st"
  );

  // XXX: Check; blindly copied from old core.
  *flag = not_un & eq;
  return sw;
}

