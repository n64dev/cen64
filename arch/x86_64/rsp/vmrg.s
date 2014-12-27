//
// arch/x86_64/rsp/vmrg.s
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
# xmm13 = vco_lo
# xmm14 = vco_hi
#

.global RSP_VMRG
.type	RSP_VMRG, @function

RSP_VMRG:

.ifdef __AVX__
  vpblendvb %xmm11, %xmm0, %xmm1, %xmm0
  vpxor %xmm13, %xmm13, %xmm13
  movdqa %xmm0, %xmm5
  vpxor %xmm14, %xmm14, %xmm14
  retq
.else
  movdqa %xmm11, %xmm2
  pand %xmm11, %xmm0
  pandn %xmm1, %xmm2
  pxor %xmm13, %xmm13
  por %xmm2, %xmm0
  pxor %xmm14, %xmm14
  movdqa %xmm0, %xmm5
  retq
.endif

.size RSP_VMRG,.-RSP_VMRG

