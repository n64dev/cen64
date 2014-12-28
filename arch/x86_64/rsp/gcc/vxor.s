//
// arch/x86_64/rsp/gcc/vxor.s
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
.globl RSP_VXOR
.def RSP_VXOR; .scl 2; .type 32; .endef
.seh_proc RSP_VXOR
.else
.global RSP_VXOR
.type	RSP_VXOR, @function
.endif

RSP_VXOR:

  pxor %xmm1, %xmm0
  movdqa %xmm0, acc_lo
  retq

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VXOR,.-RSP_VXOR
.endif

