//
// arch/x86_64/gcc/context.s
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

//
// cen64_context_restore(cen64_context *context);
//
// Loads CPU registers that were saved with cen64_context_save.
// Only CPU registers that the calling convention considers to
// be 'volatile' are restored.
//
.ifdef __MINGW__
.file "context.s"
.section .text.unlikely,"x"
.p2align 4,,15
.globl cen64_context_restore
.def cen64_context_restore; .scl 2; .type 32; .endef
.seh_proc cen64_context_restore
cen64_context_restore:
  .seh_endprologue

  # VR4300: FPU control word
  ldmxcsr 0x80(%rcx)

  # RSP: acc_lo, acc_md, acc_hi
  movdqa 0x00(%rcx), %xmm5

  retq
  .seh_endproc

.else
.section .text.unlikely,"ax",@progbits
.globl cen64_context_restore
.type cen64_context_restore, @function
cen64_context_restore:
  .cfi_startproc

  # VR4300: FPU control word
  ldmxcsr 0x80(%rdi)

  # RSP: acc_lo, acc_md, acc_hi
  movdqa 0x00(%rdi), %xmm5
  movdqa 0x10(%rdi), %xmm6
  movdqa 0x20(%rdi), %xmm7

  # RSP: vcc, vco, vce
  movdqa 0x30(%rdi), %xmm11
  movdqa 0x40(%rdi), %xmm12
  movdqa 0x50(%rdi), %xmm13
  movdqa 0x60(%rdi), %xmm14
  movdqa 0x70(%rdi), %xmm15

  retq
  .cfi_endproc

.size cen64_context_restore,.-cen64_context_restore
.endif

//
// cen64_context_save(cen64_context *context);
//
// Stores CPU registers that the calling convention considers
// to be 'volatile'. These registers can be saved at a later
// time with cen64_context_save.
//
.ifdef __MINGW__
.file "context.s"
.section .text.unlikely,"x"
.p2align 4,,15
.globl cen64_context_save
.def cen64_context_save; .scl 2; .type 32; .endef
.seh_proc cen64_context_save
cen64_context_save:
  .seh_endprologue

  # VR4300: FPU control word
  stmxcsr 0x80(%rcx)

  # RSP: acc_lo, acc_md, acc_hi
  movdqa %xmm5, 0x00(%rcx)

  retq
  .seh_endproc

.else
.section .text.unlikely,"ax",@progbits
.globl cen64_context_save
.type cen64_context_save, @function
cen64_context_save:
  .cfi_startproc

  # VR4300: FPU control word
  stmxcsr 0x80(%rdi)

  # RSP: acc_lo, acc_md, acc_hi
  movdqa %xmm5, 0x00(%rdi)
  movdqa %xmm6, 0x10(%rdi)
  movdqa %xmm7, 0x20(%rdi)

  # RSP: vcc, vco, vce
  movdqa %xmm11, 0x30(%rdi)
  movdqa %xmm12, 0x40(%rdi)
  movdqa %xmm13, 0x50(%rdi)
  movdqa %xmm14, 0x60(%rdi)
  movdqa %xmm15, 0x70(%rdi)

  retq
  .cfi_endproc

.size cen64_context_save,.-cen64_context_save
.endif

