//
// arch/x86_64/rsp/vnor.s
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
.globl RSP_VNOR
.def RSP_VNOR; .scl 2; .type 32; .endef
.seh_proc RSP_VNOR
RSP_VNOR:
.else
.global RSP_VNOR
.type	RSP_VNOR, @function
RSP_VNOR:
.endif

RSP_VNOR:
  pcmpeqd %xmm2, %xmm2
  por %xmm1, %xmm0
  pxor %xmm2, %xmm0
  movdqa %xmm0, %xmm5
  retq

.ifdef __MINGW32__
.seh_endproc
.else
.size RSP_VNOR,.-RSP_VNOR
.endif

