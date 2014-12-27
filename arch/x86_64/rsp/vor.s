//
// arch/x86_64/rsp/vor.s
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

.global RSP_VOR
.type	RSP_VOR, @function

RSP_VOR:

.ifdef __AVX__
  vpor %xmm1, %xmm0, %xmm0
  vmovdqa %xmm0, %xmm5
  retq
.else
  por %xmm1, %xmm0
  movdqa %xmm0, %xmm5
  retq
.endif

.size RSP_VOR,.-RSP_VOR

