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
# xmm1 = vs
# xmm0 = vt
# xmm5 = acc_lo
# xmm11 = vcc_lo
# xmm13 = vco_lo
# xmm14 = vco_hi
#

.ifdef __MINGW32__
.globl RSP_VMRG
.def RSP_VMRG; .scl 2; .type 32; .endef
.seh_proc RSP_VMRG
RSP_VMRG:
.else
.global RSP_VMRG
.type	RSP_VMRG, @function
RSP_VMRG:
.endif

.ifdef __AVX__
  vpblendvb %xmm11, %xmm1, %xmm0, %xmm0
  pxor %xmm13, %xmm13
  movdqa %xmm0, %xmm5
  pxor %xmm14, %xmm14
  retq

.elseif __SSE4_1__ == 1
  movdqa %xmm0, %xmm5
  movdqa %xmm11, %xmm0
  pxor %xmm13, %xmm13
  pblendvb %xmm0, %xmm1, %xmm5
  pxor %xmm14, %xmm14
  movdqa %xmm5, %xmm0
  retq

.else
  movdqa %xmm11, %xmm5
  pxor %xmm13, %xmm13
  pand %xmm5, %xmm1
  pandn %xmm0, %xmm5
  pxor %xmm14, %xmm14
  por %xmm1, %xmm5
  movdqa %xmm5, %xmm0
  retq
.endif

.ifdef __MINGW32__
.seh_endproc
.else
.size RSP_VMRG,.-RSP_VMRG
.endif

