//
// arch/x86_64/fpu/fpu.h
//
// Extern declarations for host FPU functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __arch_fpu_h__
#define __arch_fpu_h__
#include "common.h"

typedef uint16_t fpu_state_t;

#define FPU_MASK_EXCPS    0x1F80

#define FPU_ROUND_MASK    0x6000
#define FPU_ROUND_NEAREST 0x0000
#define FPU_ROUND_NEGINF  0x2000
#define FPU_ROUND_POSINF  0x4000
#define FPU_ROUND_TOZERO  0x6000

#include "arch/x86_64/fpu/abs_32.h"
#include "arch/x86_64/fpu/abs_64.h"
#include "arch/x86_64/fpu/add_32.h"
#include "arch/x86_64/fpu/add_64.h"
#include "arch/x86_64/fpu/cmp_ueq_32.h"
#include "arch/x86_64/fpu/cmp_ueq_64.h"
#include "arch/x86_64/fpu/cmp_ule_32.h"
#include "arch/x86_64/fpu/cmp_ule_64.h"
#include "arch/x86_64/fpu/cmp_ult_32.h"
#include "arch/x86_64/fpu/cmp_ult_64.h"
#include "arch/x86_64/fpu/cvt_f32_f64.h"
#include "arch/x86_64/fpu/cvt_f32_i32.h"
#include "arch/x86_64/fpu/cvt_f32_i64.h"
#include "arch/x86_64/fpu/cvt_f64_f32.h"
#include "arch/x86_64/fpu/cvt_f64_i32.h"
#include "arch/x86_64/fpu/cvt_f64_i64.h"
#include "arch/x86_64/fpu/cvt_i32_f32.h"
#include "arch/x86_64/fpu/cvt_i32_f64.h"
#include "arch/x86_64/fpu/cvt_i64_f32.h"
#include "arch/x86_64/fpu/cvt_i64_f64.h"
#include "arch/x86_64/fpu/div_32.h"
#include "arch/x86_64/fpu/div_64.h"
#include "arch/x86_64/fpu/mul_32.h"
#include "arch/x86_64/fpu/mul_64.h"
#include "arch/x86_64/fpu/neg_32.h"
#include "arch/x86_64/fpu/neg_64.h"
#include "arch/x86_64/fpu/sqrt_32.h"
#include "arch/x86_64/fpu/sqrt_64.h"
#include "arch/x86_64/fpu/sub_32.h"
#include "arch/x86_64/fpu/sub_64.h"
#include "arch/x86_64/fpu/trunc_i32_f32.h"
#include "arch/x86_64/fpu/trunc_i32_f64.h"
#include "arch/x86_64/fpu/trunc_i64_f32.h"
#include "arch/x86_64/fpu/trunc_i64_f64.h"

#endif

