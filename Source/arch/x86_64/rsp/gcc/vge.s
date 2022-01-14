//
// arch/x86_64/rsp/gcc/vge.s
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
.globl RSP_VGE
.def RSP_VGE; .scl 2; .type 32; .endef
.seh_proc RSP_VGE
.ifndef __VECTORCALL__
RSP_VGE:
  movdqa (%r8), %xmm0
  movdqa (%r9), %xmm1
  pxor %xmm2, %xmm2
.endif
.else
.global RSP_VGE
.type	RSP_VGE, @function
RSP_VGE:
.endif

.ifdef __AVX__
  vpand vco_lo, vco_hi, %xmm2
  vpcmpeqw %xmm1, %xmm0, %xmm3
  vpcmpgtw %xmm0, %xmm1, %xmm4
  pxor vco_hi, vco_hi
  pandn %xmm3, %xmm2
  pxor vco_lo, vco_lo
  vpor %xmm2, %xmm4, vcc_lo
  pxor vcc_hi, vcc_hi
  vpblendvb vcc_lo, %xmm1, %xmm0, %xmm0
  movdqa %xmm0, acc_lo
  retq

.elseif __SSE4_1__ == 1
  movdqa %xmm0, acc_lo
  movdqa %xmm0, %xmm2
  movdqa %xmm1, %xmm0
  pand vco_lo, vco_hi
  pcmpgtw %xmm2, %xmm0
  pxor vcc_hi, vcc_hi
  pcmpeqw %xmm1, %xmm2
  pandn %xmm2, vco_hi
  pxor vco_lo, vco_lo
  por vco_hi, %xmm0
  pxor vco_hi, vco_hi
  pblendvb %xmm0, %xmm1, acc_lo
  movdqa %xmm0, vcc_lo
  movdqa acc_lo, %xmm0
  retq

.else
  movdqa %xmm1, acc_lo
  movdqa %xmm1, %xmm2
  pand vco_lo, vco_hi
  pcmpgtw %xmm0, %xmm1
  pcmpeqw %xmm0, %xmm2
  pxor vcc_hi, vcc_hi
  pandn %xmm2, vco_hi
  pxor vco_lo, vco_lo
  por vco_hi, %xmm1
  movdqa %xmm1,vcc_lo
  pand %xmm1, acc_lo
  pandn %xmm0, %xmm1
  por %xmm1, acc_lo
  movdqa acc_lo, %xmm0
  pxor vco_hi, vco_hi
  retq
.endif

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VGE,.-RSP_VGE
.endif

