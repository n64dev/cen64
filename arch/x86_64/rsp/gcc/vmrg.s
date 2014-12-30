//
// arch/x86_64/rsp/gcc/vmrg.s
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

.include "rsp/gcc/defs.h"

.text

.ifdef __MINGW__
.globl RSP_VMRG
.def RSP_VMRG; .scl 2; .type 32; .endef
.seh_proc RSP_VMRG
.ifndef __VECTORCALL__
RSP_VMRG:
  movdqa (%r8), %xmm0
  movdqa (%r9), %xmm1
  #pxor %xmm2, %xmm2
.endif
.else
.global RSP_VMRG
.type	RSP_VMRG, @function
RSP_VMRG:
.endif

.ifdef __AVX__
  vpblendvb vcc_lo, %xmm1, %xmm0, %xmm0
  pxor vco_lo, vco_lo
  movdqa %xmm0, acc_lo
  pxor vco_hi, vco_hi
  retq

.elseif __SSE4_1__ == 1
  movdqa %xmm0, acc_lo
  movdqa vcc_lo, %xmm0
  pxor vco_lo, vco_lo
  pblendvb %xmm0, %xmm1, acc_lo
  pxor vco_hi, vco_hi
  movdqa acc_lo, %xmm0
  retq

.else
  movdqa vcc_lo, acc_lo
  pxor vco_lo, vco_lo
  pand acc_lo, %xmm1
  pandn %xmm0, acc_lo
  pxor vco_hi, vco_hi
  por %xmm1, acc_lo
  movdqa acc_lo, %xmm0
  retq
.endif

.ifdef __MINGW__
.seh_endproc
.else
.size RSP_VMRG,.-RSP_VMRG
.endif

