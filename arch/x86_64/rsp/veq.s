//
// arch/x86_64/rsp/veq.s
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

.global RSP_VEQ
.type	RSP_VEQ, @function

RSP_VEQ:

.ifdef __AVX__
  vpcmpeqw %xmm0, %xmm1, %xmm11
  vpxor %xmm12, %xmm12, %xmm12
  vpandn %xmm11, %xmm14, %xmm11
  vpxor %xmm13, %xmm13, %xmm13
  vpblendvb %xmm11, %xmm1, %xmm0, %xmm1
  vpxor %xmm14, %xmm14, %xmm14
  movdqa %xmm1, %xmm5
  retq
.elseif __SSE4_1__ == 1
  movdqa %xmm0, %xmm5
  pcmpeqw %xmm1, %xmm0
  pxor %xmm12, %xmm12
  pandn %xmm0, %xmm14
  pxor %xmm13, %xmm13
  movdqa %xmm14, %xmm0
  pxor %xmm14, %xmm14
  pblendvb %xmm0, %xmm1, %xmm5
  movdqa %xmm0, %xmm11
  movdqa %xmm5, %xmm0
  retq
.else
  movdqa %xmm1, %xmm5
  pcmpeqw %xmm0, %xmm1
  pxor %xmm12, %xmm12
  pandn %xmm1, %xmm14
  pxor %xmm13, %xmm13
  movdqa %xmm14, %xmm11
  pand %xmm14, %xmm5
  pandn %xmm0, %xmm14
  por %xmm14, %xmm5
  pxor %xmm14, %xmm14
  movdqa %xmm5, %xmm1
  retq
.endif

.size RSP_VEQ,.-RSP_VEQ

