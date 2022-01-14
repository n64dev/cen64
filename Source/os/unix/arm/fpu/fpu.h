//
// os/unix/arm/fpu/fpu.h
//
// Extern declarations for host FPU functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_fpu_h__
#define __os_fpu_h__
#include "common.h"

#include "arch/arm/fpu/fpu.h"

static inline fpu_state_t fpu_get_state(void) {
  fpu_state_t state;

  __asm__ __volatile__("mrc p10, 7, %0, cr1, cr0, 0\n\t" : "=r"(state));
  return state;
}

static inline void fpu_set_state(fpu_state_t state) {
  __asm__ __volatile__("mcr p10, 7, %0, cr1, cr0, 0\n\t" :: "r"(state));
}

#endif

