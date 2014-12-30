//
// arch/x86_64/rsp/gcc/vand.s
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

.include "rsp/gcc/defs.h"

.text

.ifdef __MINGW__
.globl RSP_VAND
.def RSP_VAND; .scl 2; .type 32; .endef
.seh_proc RSP_VAND
.ifndef __VECTORCALL__
RSP_VAND:
  movdqa (%r8), %xmm0
  movdqa (%r9), %xmm1
  #pxor %xmm2, %xmm2
.endif
.else
.global RSP_VAND
.type	RSP_VAND, @function
RSP_VAND:
.endif

  pand %xmm1, %xmm0
  movdqa %xmm0, acc_lo
  retq

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VAND,.-RSP_VAND
.endif

