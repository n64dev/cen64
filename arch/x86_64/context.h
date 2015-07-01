//
// arch/x86_64/context.h
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_ARCH_X86_64_CONTEXT_H
#define CEN64_ARCH_X86_64_CONTEXT_H
#include "common.h"
#include <emmintrin.h>

struct cen64_context {
  __m128i regs[8];
  uint32_t mxcsr;
};

cen64_cold void cen64_context_restore(struct cen64_context *context);
cen64_cold void cen64_context_save(struct cen64_context *context);

#endif

