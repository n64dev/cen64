//
// arch/x86_64/rsp/gcc/vlt.s
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
.globl RSP_VLT
.def RSP_VLT; .scl 2; .type 32; .endef
.seh_proc RSP_VLT
.else
.global RSP_VLT
.type	RSP_VLT, @function
.endif

RSP_VLT:

.ifdef __AVX__
  vpcmpeqw %xmm1, %xmm0, %xmm3
  vpcmpgtw %xmm1, %xmm0, %xmm4
  pxor vcc_hi, vcc_hi
  vpandn %xmm3, vco_hi, %xmm3
  pxor vco_hi, vco_hi
  pand vco_lo, %xmm3
  vpor %xmm3, %xmm4, vcc_lo
  pxor vco_lo, vco_lo
  vpblendvb vcc_lo, %xmm1, %xmm0, %xmm0
  movdqa %xmm0, acc_lo
  retq

.else
  movdqa %xmm1, %xmm3
  movdqa %xmm0, vcc_lo
  pcmpeqw %xmm0, %xmm3
  pcmpgtw %xmm1, vcc_lo
  pandn vco_lo, vco_hi
  pminsw %xmm1, %xmm0
  pand %xmm3, vco_hi
  pxor vcc_hi, vcc_hi
  movdqa %xmm0, acc_lo
  por vco_hi, vcc_lo
  pxor vco_lo, vco_lo
  pxor vco_hi, vco_hi
  retq
.endif

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VLT,.-RSP_VLT
.endif

