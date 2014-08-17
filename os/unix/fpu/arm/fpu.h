//
// os/unix/fpu/arm/fpu.h
//
// Extern declarations for host FPU functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_fpu_h__
#define __os_fpu_h__
#include "common.h"

typedef unsigned int fpu_state_t;

#define FPU_MASK_EXCPS    0x1F80

#define FPU_ROUND_MASK    0x6000
#define FPU_ROUND_NEAREST 0x0000
#define FPU_ROUND_NEGINF  0x2000
#define FPU_ROUND_POSINF  0x4000
#define FPU_ROUND_TOZERO  0x6000

static inline fpu_state_t fpu_get_state(void) {
  fpu_state_t state;

  __asm__ __volatile__("mrc p10, 7, %0, cr1, cr0, 0\n\t" : "=r"(state));
  return state;
}

static inline void fpu_set_state(fpu_state_t state) {
  __asm__ __volatile__("mcr p10, 7, %0, cr1, cr0, 0\n\t" :: "r"(state));
}

#include "os/unix/fpu/arm/fpu_abs_32.h"
#include "os/unix/fpu/arm/fpu_abs_64.h"
#include "os/unix/fpu/arm/fpu_add_32.h"
#include "os/unix/fpu/arm/fpu_add_64.h"
#include "os/unix/fpu/arm/fpu_cmp_eq_32.h"
#include "os/unix/fpu/arm/fpu_cmp_eq_64.h"
#include "os/unix/fpu/arm/fpu_cmp_f_32.h"
#include "os/unix/fpu/arm/fpu_cmp_f_64.h"
#include "os/unix/fpu/arm/fpu_cmp_ole_32.h"
#include "os/unix/fpu/arm/fpu_cmp_ole_64.h"
#include "os/unix/fpu/arm/fpu_cmp_olt_32.h"
#include "os/unix/fpu/arm/fpu_cmp_olt_64.h"
#include "os/unix/fpu/arm/fpu_cmp_ueq_32.h"
#include "os/unix/fpu/arm/fpu_cmp_ueq_64.h"
#include "os/unix/fpu/arm/fpu_cmp_ule_32.h"
#include "os/unix/fpu/arm/fpu_cmp_ule_64.h"
#include "os/unix/fpu/arm/fpu_cmp_ult_32.h"
#include "os/unix/fpu/arm/fpu_cmp_ult_64.h"
#include "os/unix/fpu/arm/fpu_cmp_un_32.h"
#include "os/unix/fpu/arm/fpu_cmp_un_64.h"
#include "os/unix/fpu/arm/fpu_cvt_f32_f64.h"
#include "os/unix/fpu/arm/fpu_cvt_f32_i32.h"
#include "os/unix/fpu/arm/fpu_cvt_f32_i64.h"
#include "os/unix/fpu/arm/fpu_cvt_f64_f32.h"
#include "os/unix/fpu/arm/fpu_cvt_f64_i32.h"
#include "os/unix/fpu/arm/fpu_cvt_f64_i64.h"
#include "os/unix/fpu/arm/fpu_cvt_i32_f32.h"
#include "os/unix/fpu/arm/fpu_cvt_i32_f64.h"
#include "os/unix/fpu/arm/fpu_cvt_i64_f32.h"
#include "os/unix/fpu/arm/fpu_cvt_i64_f64.h"
#include "os/unix/fpu/arm/fpu_div_32.h"
#include "os/unix/fpu/arm/fpu_div_64.h"
#include "os/unix/fpu/arm/fpu_mul_32.h"
#include "os/unix/fpu/arm/fpu_mul_64.h"
#include "os/unix/fpu/arm/fpu_neg_32.h"
#include "os/unix/fpu/arm/fpu_neg_64.h"
#include "os/unix/fpu/arm/fpu_sqrt_32.h"
#include "os/unix/fpu/arm/fpu_sqrt_64.h"
#include "os/unix/fpu/arm/fpu_sub_32.h"
#include "os/unix/fpu/arm/fpu_sub_64.h"
#include "os/unix/fpu/arm/fpu_trunc_i32_f32.h"
#include "os/unix/fpu/arm/fpu_trunc_i32_f64.h"
#include "os/unix/fpu/arm/fpu_trunc_i64_f32.h"
#include "os/unix/fpu/arm/fpu_trunc_i64_f64.h"

#endif

