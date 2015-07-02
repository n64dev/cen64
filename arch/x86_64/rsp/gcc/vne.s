//
// arch/x86_64/rsp/gcc/vne.s
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
.globl RSP_VNE
.def RSP_VNE; .scl 2; .type 32; .endef
.seh_proc RSP_VNE
.ifndef __VECTORCALL__
RSP_VNE:
  movdqa (%r8), %xmm0
  movdqa (%r9), %xmm1
  pxor %xmm2, %xmm2
.endif
.else
.global RSP_VNE
.type	RSP_VNE, @function
RSP_VNE:
.endif

.ifdef __AVX__
  vpcmpeqw %xmm0, %xmm1, %xmm3
  pcmpeqw %xmm3, %xmm2
  pxor vcc_hi, vcc_hi
  pand vco_hi, %xmm3
  vpor %xmm3, %xmm2, vcc_lo
  pxor vco_lo, vco_lo
  vpblendvb vcc_lo, %xmm1, %xmm0, %xmm0
  pxor vco_hi, vco_hi
  movdqa %xmm0, acc_lo
  retq

.elseif __SSE4_1__ == 1
  movdqa %xmm0, acc_lo
  pcmpeqw %xmm1, %xmm0
  pcmpeqw %xmm0, %xmm2
  pxor vcc_hi, vcc_hi
  pand vco_hi, %xmm0
  pxor vco_lo, vco_lo
  por %xmm2, %xmm0
  pxor vco_hi, vco_hi
  pblendvb %xmm0, %xmm1, acc_lo
  movdqa %xmm0, vcc_lo
  movdqa acc_lo, %xmm0
  retq

.else
  movdqa %xmm0, %xmm3
  pcmpeqw %xmm1, %xmm0
  pcmpeqw %xmm0, %xmm2
  pxor vcc_hi, vcc_hi
  pand vco_hi, %xmm0
  pxor vco_lo, vco_lo
  por %xmm2, %xmm0
  pxor vco_hi, vco_hi
  pand %xmm0, %xmm1
  movdqa %xmm0, vcc_lo
  pandn %xmm3, %xmm0
  por %xmm1, %xmm0
  movdqa %xmm0, acc_lo
  retq
.endif

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VNE,.-RSP_VNE
.endif

