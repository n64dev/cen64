//
// arch/x86_64/rsp/vlt.s
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#
# xmm1 = vs
# xmm0 = vt
# xmm5 = acc_lo
# xmm11 = vcc_lo
# xmm12 = vcc_hi
# xmm13 = vco_lo
# xmm14 = vco_hi
#

.ifdef __MINGW32__
.globl RSP_VLT
.def RSP_VLT; .scl 2; .type 32; .endef
.seh_proc RSP_VLT
RSP_VLT:
.else
.global RSP_VLT
.type	RSP_VLT, @function
RSP_VLT:
.endif

.ifdef __AVX__
  vpcmpeqw %xmm1, %xmm0, %xmm3
  vpcmpgtw %xmm1, %xmm0, %xmm4
  pxor %xmm14, %xmm14
  vpandn %xmm3, %xmm14, %xmm3
  pxor %xmm13, %xmm13
  vpand %xmm3, %xmm13, %xmm3
  vpor %xmm3, %xmm4, %xmm11
  pxor %xmm12, %xmm12
  vpblendvb %xmm11, %xmm1, %xmm0, %xmm0
  movdqa %xmm0, %xmm5
  retq

.elseif __SSE4_1__ == 1
  movdqa %xmm0, %xmm5
  movdqa %xmm0, %xmm2
  pcmpgtw %xmm1, %xmm0
  pcmpeqw %xmm1, %xmm2
  pxor %xmm12, %xmm12
  pandn %xmm2, %xmm14
  pxor %xmm13, %xmm13
  pand %xmm14, %xmm13
  pxor %xmm14, %xmm14
  por %xmm13, %xmm0
  pblendvb %xmm0, %xmm1, %xmm5
  movdqa %xmm0, %xmm11
  movdqa %xmm5, %xmm0
  retq

.else
  movdqa %xmm0, %xmm5
  movdqa %xmm0, %xmm2
  pcmpgtw %xmm1, %xmm0
  pcmpeqw %xmm1, %xmm2
  pxor %xmm12, %xmm12
  pandn %xmm2, %xmm14
  pxor %xmm13, %xmm13
  pand %xmm14, %xmm13
  pxor %xmm14, %xmm14
  por %xmm13, %xmm0
  movdqa %xmm0,%xmm11
  pand %xmm0, %xmm1
  pandn %xmm5, %xmm0
  por %xmm1, %xmm0
  movdqa %xmm0, %xmm5
  retq
.endif

.ifdef __MINGW32__
.seh_endproc
.else
.size RSP_VLT,.-RSP_VLT
.endif

