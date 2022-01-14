//
// arch/x86_64/rsp/gcc/vnxor.s
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

.include "rsp/gcc/defs.h"

.text

.ifdef __MINGW__
.globl RSP_VNXOR
.def RSP_VNXOR; .scl 2; .type 32; .endef
.seh_proc RSP_VNXOR
.ifndef __VECTORCALL__
RSP_VNXOR:
  movdqa (%r8), %xmm0
  movdqa (%r9), %xmm1
  pxor %xmm2, %xmm2
.endif
.else
.global RSP_VNXOR
.type	RSP_VNXOR, @function
RSP_VNXOR:
.endif

  pcmpeqd %xmm2, %xmm2
  pxor %xmm1, %xmm0
  pxor %xmm2, %xmm0
  movdqa %xmm0, acc_lo
  retq

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VNXOR,.-RSP_VNXOR
.endif

