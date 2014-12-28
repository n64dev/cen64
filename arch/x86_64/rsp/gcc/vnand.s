//
// arch/x86_64/rsp/gcc/vnand.s
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
.globl RSP_VNAND
.def RSP_VNAND; .scl 2; .type 32; .endef
.seh_proc RSP_VNAND
.else
.global RSP_VNAND
.type	RSP_VMRG, @function
.endif

RSP_VNAND:

  pcmpeqd %xmm2, %xmm2
  pand %xmm1, %xmm0
  pxor %xmm2, %xmm0
  movdqa %xmm0, acc_lo
  retq

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VNAND,.-RSP_VNAND
.endif

