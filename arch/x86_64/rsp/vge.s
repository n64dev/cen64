//
// arch/x86_64/rsp/vge.s
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#
# xmm0 = vs
# xmm1 = vt
# xmm5 = acc_lo
# xmm11 = vcc_lo
# xmm12 = vcc_hi
# xmm13 = vco_lo
# xmm14 = vco_hi
#

.global RSP_VGE
.type	RSP_VGE, @function

RSP_VGE:

.ifdef __AVX__
  vpand %xmm13, %xmm14, %xmm2
  vpcmpeqw %xmm0, %xmm1, %xmm3
  vpcmpgtw %xmm1, %xmm0, %xmm4
  pxor %xmm14, %xmm14
  vpandn %xmm3, %xmm2, %xmm3
  pxor %xmm13, %xmm13
  vpor %xmm3, %xmm4, %xmm11
  pxor %xmm12, %xmm12
  vpblendvb %xmm11, %xmm0, %xmm1, %xmm0
  movdqa %xmm0, %xmm5
  retq
.else
  movdqa %xmm0, %xmm2
  pand %xmm13, %xmm14
  movdqa %xmm1, %xmm5
  pcmpgtw %xmm1, %xmm0
  pcmpeqw %xmm2, %xmm1
  pxor %xmm12, %xmm12
  pandn %xmm1, %xmm14
  pxor %xmm13, %xmm13
  por %xmm14, %xmm0
  movdqa %xmm0,%xmm11
.ifdef __SSE4_1__
  pblendvb %xmm0, %xmm2, %xmm5
  movdqa %xmm5, %xmm0
.else
  pand %xmm0, %xmm2
  pandn %xmm5, %xmm0
  por %xmm2, %xmm0
  movdqa %xmm0, %xmm5
.endif
  pxor %xmm14, %xmm14
  retq
.endif

.size RSP_VGE,.-RSP_VGE

