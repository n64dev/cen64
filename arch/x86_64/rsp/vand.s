//
// arch/x86_64/rsp/vand.s
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

.text

.ifdef __MINGW32__
.globl RSP_VAND
.def RSP_VAND; .scl 2; .type 32; .endef
.seh_proc RSP_VAND
RSP_VAND:
.else
.global RSP_VAND
.type	RSP_VAND, @function
RSP_VAND:
.endif

  pand %xmm1, %xmm0
  movdqa %xmm0, %xmm5
  retq

.ifdef __MINGW32__
.seh_endproc
.else
.size RSP_VAND,.-RSP_VAND
.endif

