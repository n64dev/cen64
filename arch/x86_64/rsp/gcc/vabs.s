//
// arch/x86_64/rsp/gcc/vabs.s
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
.globl RSP_VABS
.def RSP_VABS; .scl 2; .type 32; .endef
.seh_proc RSP_VABS
.else
.global RSP_VABS
.type	RSP_VABS, @function
.endif

RSP_VABS:

.ifdef __AVX__
  vpsraw $0xf, %xmm1, %xmm3
  vpxor %xmm3, %xmm0, %xmm2
  vpsubsw %xmm3, %xmm2, %xmm2
  vpsignw %xmm1, %xmm0, acc_lo
  vpblendvb %xmm3, %xmm2, acc_lo, %xmm0
  retq

.elseif __SSSE3__ == 1
  psignw %xmm1, %xmm0
  psraw $0xF, %xmm1
  movdqa %xmm0, %xmm5
  paddw %xmm1, %xmm0
  psubsw %xmm1, %xmm0
  retq

.else
  pcmpeqw %xmm1, %xmm2
  psraw $0xF, %xmm1
  pandn %xmm0, %xmm2
  pxor %xmm1, %xmm2
  movdqa %xmm2, acc_lo
  psubsw %xmm1, %xmm2
  psubw %xmm1, acc_lo
  movdqa %xmm2, %xmm0
  retq
.endif

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VABS,.-RSP_VABS
.endif

