//
// os/windows/x86_64/fpu/fpu.h
//
// Extern declarations for host FPU functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_fpu_h__
#define __os_fpu_h__
#include "common.h"
#include <emmintrin.h>

#include "arch/x86_64/fpu/fpu.h"

uint8_t fpu_cmp_eq_32(const uint32_t *fs, const uint32_t *ft);
uint8_t fpu_cmp_eq_64(const uint64_t *fs, const uint64_t *ft);
uint8_t fpu_cmp_f_32(const uint32_t *fs, const uint32_t *ft);
uint8_t fpu_cmp_f_64(const uint64_t *fs, const uint64_t *ft);
uint8_t fpu_cmp_ole_32(const uint32_t *fs, const uint32_t *ft);
uint8_t fpu_cmp_ole_64(const uint64_t *fs, const uint64_t *ft);
uint8_t fpu_cmp_olt_32(const uint32_t *fs, const uint32_t *ft);
uint8_t fpu_cmp_olt_64(const uint64_t *fs, const uint64_t *ft);
uint8_t fpu_cmp_un_32(const uint32_t *fs, const uint32_t *ft);
uint8_t fpu_cmp_un_64(const uint64_t *fs, const uint64_t *ft);

#endif

