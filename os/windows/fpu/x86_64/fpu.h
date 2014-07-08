//
// os/windows/fpu/x86_64/fpu.h
//
// Extern declarations for host FPU functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_fpu_h__
#define __os_fpu_h__
#include "common.h"

extern uint16_t fpu_abs_32(uint32_t *fs, uint32_t *fd);
extern uint16_t fpu_abs_64(uint64_t *fs, uint64_t *fd);
extern uint16_t fpu_add_32(uint32_t *fs, uint32_t *ft, uint32_t *fd);
extern uint16_t fpu_add_64(uint64_t *fs, uint64_t *ft, uint64_t *fd);
extern uint16_t fpu_cvt_f32_f64(uint64_t *fs, uint32_t *fd);
extern uint16_t fpu_cvt_f32_i32(uint32_t *fs, uint32_t *fd);
extern uint16_t fpu_cvt_f32_i64(uint64_t *fs, uint32_t *fd);
extern uint16_t fpu_cvt_f64_f32(uint32_t *fs, uint64_t *fd);
extern uint16_t fpu_cvt_f64_i32(uint32_t *fs, uint64_t *fd);
extern uint16_t fpu_cvt_f64_i64(uint64_t *fs, uint64_t *fd);
extern uint16_t fpu_cvt_i32_f32(uint32_t *fs, uint32_t *fd);
extern uint16_t fpu_cvt_i32_f64(uint64_t *fs, uint32_t *fd);
extern uint16_t fpu_cvt_i64_f32(uint32_t *fs, uint64_t *fd);
extern uint16_t fpu_cvt_i64_f64(uint64_t *fs, uint64_t *fd);
extern uint16_t fpu_div_32(uint32_t *fs, uint32_t *ft, uint32_t *fd);
extern uint16_t fpu_div_64(uint64_t *fs, uint64_t *ft, uint64_t *fd);
extern uint16_t fpu_mul_32(uint32_t *fs, uint32_t *ft, uint32_t *fd);
extern uint16_t fpu_mul_64(uint64_t *fs, uint64_t *ft, uint64_t *fd);
extern uint16_t fpu_neg_32(uint32_t *fs, uint32_t *fd);
extern uint16_t fpu_neg_64(uint64_t *fs, uint64_t *fd);
extern uint16_t fpu_round_i32_f32(uint32_t *fs, uint32_t *fd);
extern uint16_t fpu_round_i32_f64(uint64_t *fs, uint32_t *fd);
extern uint16_t fpu_round_i64_f32(uint32_t *fs, uint64_t *fd);
extern uint16_t fpu_round_i64_f64(uint64_t *fs, uint64_t *fd);
extern uint16_t fpu_sqrt_32(uint32_t *fs, uint32_t *fd);
extern uint16_t fpu_sqrt_64(uint64_t *fs, uint64_t *fd);
extern uint16_t fpu_sub_32(uint32_t *fs, uint32_t *ft, uint32_t *fd);
extern uint16_t fpu_sub_64(uint64_t *fs, uint64_t *ft, uint64_t *fd);
extern uint16_t fpu_trunc_i32_f32(uint32_t *fs, uint32_t *fd);
extern uint16_t fpu_trunc_i32_f64(uint64_t *fs, uint32_t *fd);
extern uint16_t fpu_trunc_i64_f32(uint32_t *fs, uint64_t *fd);
extern uint16_t fpu_trunc_i64_f64(uint64_t *fs, uint64_t *fd);

#endif

