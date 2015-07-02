//
// arch/x86_64/rsp/gcc/vch.s
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
.globl RSP_VCH
.def RSP_VCH; .scl 2; .type 32; .endef
.seh_proc RSP_VCH
.ifndef __VECTORCALL__
RSP_VCH:
  movdqa (%r8), %xmm0
  movdqa (%r9), %xmm1
  pxor %xmm2, %xmm2
.endif
.else
.global RSP_VCH
.type	RSP_VCH, @function
RSP_VCH:
.endif

.ifdef __AVX__
  vpxor %xmm0, %xmm1, vco_lo
  psraw $0xF, vco_lo
  vpxor %xmm0, vco_lo, %xmm3
  psubw vco_lo, %xmm3
  vpsubw %xmm3, %xmm1, %xmm4
  psraw $0xF, %xmm0
  vpcmpeqw %xmm4, %xmm2, acc_lo

  # vce
  vpcmpeqw vco_lo, %xmm4, vce
  pand vco_lo, vce

  # !eq
  vpor acc_lo, vce, vco_hi
  pcmpeqw %xmm2, vco_hi

  # le/ge
  pcmpgtw %xmm2, %xmm4
  por %xmm4, acc_lo
  vpblendvb vco_lo, %xmm0, acc_lo, vcc_hi
  pcmpeqw %xmm2, %xmm4
  vpblendvb vco_lo, %xmm4, %xmm0, vcc_lo

  # vd
  vpblendvb vco_lo, vcc_lo, vcc_hi, %xmm2
  vpblendvb %xmm2, %xmm3, %xmm1, %xmm0
  movdqa %xmm0, acc_lo
  retq

.elseif __SSE4_1__ == 1
  movdqa %xmm1, acc_lo
  movdqa %xmm0, vcc_lo
  movdqa %xmm0, %xmm3
  pxor %xmm1, %xmm0
  psraw $0xF, %xmm0
  pxor %xmm0, %xmm3
  psubw %xmm0, %xmm3
  psubw %xmm3, %xmm1
  pxor %xmm4, %xmm4
  psraw $0xF, vcc_lo
  pcmpeqw %xmm1, %xmm4

  # vce
  movdqa %xmm0, vce
  pcmpeqw %xmm1, vce
  pand %xmm1, vce

  # !eq
  movdqa vce, vco_hi
  por %xmm4, vco_hi
  pcmpeqw %xmm2, vco_hi

  # le/ge
  pcmpgtw %xmm2, %xmm1
  por %xmm1, %xmm4
  movdqa %xmm4, vcc_hi
  pblendvb %xmm0, vcc_lo, vcc_hi
  pcmpeqw %xmm2, %xmm1
  pblendvb %xmm0, %xmm1, vcc_lo

  # vd
  movdqa %xmm0, vco_lo
  pblendvb %xmm0, vcc_lo, %xmm4
  movdqa %xmm4, %xmm0
  pblendvb %xmm0, %xmm3, acc_lo
  movdqa acc_lo, %xmm0
  retq

.else
  movdqa %xmm0, vco_lo
  movdqa %xmm1, vce
  pxor %xmm1, vco_lo
  movdqa %xmm1, vco_hi
  psraw $0xF, vco_lo
  movdqa %xmm0, acc_lo
  movdqa %xmm1, vcc_lo
  pxor vco_lo, acc_lo
  movdqa %xmm1, vcc_hi
  pcmpeqw acc_lo, vce
  pand vco_lo, vcc_lo
  por vco_lo, vcc_hi
  psubw vco_lo, acc_lo
  pand vco_lo, vce
  paddw %xmm0, vcc_lo
  pcmpeqw acc_lo, vco_hi
  pminsw %xmm0, vcc_hi
  paddsw vco_lo, vcc_lo
  por vce, vco_hi
  psraw $0xF, vcc_lo
  pcmpeqw %xmm0, vcc_hi
  movdqa vcc_lo, %xmm3
  pcmpeqw %xmm2, vco_hi
  psubw vcc_hi, %xmm3
  psubw %xmm1, acc_lo
  pand vco_lo, %xmm3
  paddw vcc_hi, %xmm3
  pand %xmm3, acc_lo
  paddw %xmm1, acc_lo
  movdqa acc_lo, %xmm0
  retq
.endif

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VCH,.-RSP_VCH
.endif

