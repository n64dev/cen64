//
// arch/x86_64/rsp/vnand.s
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
# xmm2 = zero
# xmm5 = acc_lo
#

.global RSP_VNAND
.type	RSP_VNAND, @function

RSP_VNAND:
  pcmpeqd %xmm2, %xmm2
  pand %xmm1, %xmm0
  pxor %xmm2, %xmm0
  movdqa %xmm0, %xmm5
  retq

.size RSP_VNAND,.-RSP_VNAND

