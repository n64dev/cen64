//
// arch/x86_64/rsp/gcc/veq.s
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
.globl RSP_VEQ
.def RSP_VEQ; .scl 2; .type 32; .endef
.seh_proc RSP_VEQ
.ifndef __VECTORCALL__
RSP_VEQ:
  movdqa (%r8), %xmm0
  movdqa (%r9), %xmm1
  #pxor %xmm2, %xmm2
.endif
.else
.global RSP_VEQ
.type	RSP_VEQ, @function
RSP_VEQ:
.endif

.ifdef __AVX__
  vpcmpeqw %xmm0, %xmm1, vcc_lo
  pxor vcc_hi, vcc_hi
  vpandn vcc_lo, vco_hi, vcc_lo
  pxor vco_lo, vco_lo
  vpblendvb vcc_lo, %xmm1, %xmm0, %xmm0
  pxor vco_hi, vco_hi
  movdqa %xmm0, acc_lo
  retq

.elseif __SSE4_1__ == 1
  movdqa %xmm0, acc_lo
  pcmpeqw %xmm1, %xmm0
  pxor vcc_hi, vcc_hi
  pandn %xmm0, vco_hi
  pxor vco_lo, vco_lo
  movdqa vco_hi, %xmm0
  pxor vco_hi, vco_hi
  pblendvb %xmm0, %xmm1, acc_lo
  movdqa %xmm0, vcc_lo
  movdqa acc_lo, %xmm0
  retq

.else
  movdqa %xmm1, acc_lo
  pcmpeqw %xmm0, %xmm1
  pxor vcc_hi, vcc_hi
  pandn %xmm1, vco_hi
  pxor vco_lo, vco_lo
  movdqa vco_hi, vcc_lo
  pand vco_hi, acc_lo
  pandn %xmm0, vco_hi
  por vco_hi, acc_lo
  pxor vco_hi, vco_hi
  movdqa acc_lo, %xmm0
  retq
.endif

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VEQ,.-RSP_VEQ
.endif

