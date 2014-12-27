//
// arch/x86_64/rsp/vxor.s
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
#

.global RSP_VXOR
.type	RSP_VXOR, @function

RSP_VXOR:

.ifdef __AVX__
  vpxor %xmm1, %xmm0, %xmm0
  vmovdqa %xmm0, %xmm5
  retq
.else
  pxor %xmm1, %xmm0
  movdqa %xmm0, %xmm5
  retq
.endif

.size RSP_VXOR,.-RSP_VXOR

