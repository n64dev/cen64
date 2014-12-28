//
// arch/x86_64/rsp/vabs.s
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
#

.global RSP_VABS
.type	RSP_VABS, @function

RSP_VABS:

.ifdef __AVX__
  vpsraw $0xf, %xmm1, %xmm3
  vpxor %xmm3, %xmm0, %xmm2
  vpsubsw %xmm3, %xmm2, %xmm2
  vpsignw %xmm1, %xmm0, %xmm5
  vpblendvb %xmm3, %xmm2, %xmm5, %xmm0
  retq

.else
  pcmpeqw %xmm1, %xmm2
  psraw $0xF, %xmm1
  pandn %xmm0, %xmm2
  pxor %xmm1, %xmm2
  movdqa %xmm2, %xmm5
  psubsw %xmm1, %xmm2
  psubw %xmm1, %xmm5
  movdqa %xmm2, %xmm0
  retq
.endif

.size RSP_VABS,.-RSP_VABS

