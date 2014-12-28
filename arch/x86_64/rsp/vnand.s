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

.ifdef __MINGW32__
.globl RSP_VNAND
.def RSP_VNAND; .scl 2; .type 32; .endef
.seh_proc RSP_VNAND
RSP_VNAND:
.else
.global RSP_VNAND
.type	RSP_VMRG, @function
RSP_VNAND:
.endif

  pcmpeqd %xmm2, %xmm2
  pand %xmm1, %xmm0
  pxor %xmm2, %xmm0
  movdqa %xmm0, %xmm5
  retq

.ifdef __MINGW32__
.seh_endproc
.else
.size RSP_VNAND,.-RSP_VNAND
.endif

