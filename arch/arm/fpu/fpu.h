//
// arch/arm/fpu/fpu.h
//
// Extern declarations for host FPU functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __arch_fpu_h__
#define __arch_fpu_h__
#include "common.h"

typedef uint32_t fpu_state_t;

//
// TODO: Copied from x86_64. Likely not right.
//
#define FPU_MASK_EXCPS    0x1F80

#define FPU_ROUND_MASK    0x6000
#define FPU_ROUND_NEAREST 0x0000
#define FPU_ROUND_NEGINF  0x2000
#define FPU_ROUND_POSINF  0x4000
#define FPU_ROUND_TOZERO  0x6000

#include "arch/arm/fpu/abs_32.h"
#include "arch/arm/fpu/abs_64.h"
#include "arch/arm/fpu/add_32.h"
#include "arch/arm/fpu/add_64.h"
#include "arch/arm/fpu/cmp_eq_32.h"
#include "arch/arm/fpu/cmp_eq_64.h"
#include "arch/arm/fpu/cmp_f_32.h"
#include "arch/arm/fpu/cmp_f_64.h"
#include "arch/arm/fpu/cmp_ole_32.h"
#include "arch/arm/fpu/cmp_ole_64.h"
#include "arch/arm/fpu/cmp_olt_32.h"
#include "arch/arm/fpu/cmp_olt_64.h"
#include "arch/arm/fpu/cmp_ueq_32.h"
#include "arch/arm/fpu/cmp_ueq_64.h"
#include "arch/arm/fpu/cmp_ule_32.h"
#include "arch/arm/fpu/cmp_ule_64.h"
#include "arch/arm/fpu/cmp_ult_32.h"
#include "arch/arm/fpu/cmp_ult_64.h"
#include "arch/arm/fpu/cmp_un_32.h"
#include "arch/arm/fpu/cmp_un_64.h"
#include "arch/arm/fpu/cvt_f32_f64.h"
#include "arch/arm/fpu/cvt_f32_i32.h"
#include "arch/arm/fpu/cvt_f32_i64.h"
#include "arch/arm/fpu/cvt_f64_f32.h"
#include "arch/arm/fpu/cvt_f64_i32.h"
#include "arch/arm/fpu/cvt_f64_i64.h"
#include "arch/arm/fpu/cvt_i32_f32.h"
#include "arch/arm/fpu/cvt_i32_f64.h"
#include "arch/arm/fpu/cvt_i64_f32.h"
#include "arch/arm/fpu/cvt_i64_f64.h"
#include "arch/arm/fpu/div_32.h"
#include "arch/arm/fpu/div_64.h"
#include "arch/arm/fpu/mul_32.h"
#include "arch/arm/fpu/mul_64.h"
#include "arch/arm/fpu/neg_32.h"
#include "arch/arm/fpu/neg_64.h"
#include "arch/arm/fpu/sqrt_32.h"
#include "arch/arm/fpu/sqrt_64.h"
#include "arch/arm/fpu/sub_32.h"
#include "arch/arm/fpu/sub_64.h"
#include "arch/arm/fpu/trunc_i32_f32.h"
#include "arch/arm/fpu/trunc_i32_f64.h"
#include "arch/arm/fpu/trunc_i64_f32.h"
#include "arch/arm/fpu/trunc_i64_f64.h"

#endif

