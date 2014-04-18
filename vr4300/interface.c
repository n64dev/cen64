//
// vr4300/interface.c: VR4300 interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "vr4300/cpu.h"
#include "vr4300/interface.h"

// Reads a word from the MI MMIO register space.
int read_mi_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct vr4300 *vr4300 = (struct vr4300 *) opaque;
  uint32_t offset = address - MI_REGS_BASE_ADDRESS;
  enum mi_register reg = (offset >> 2);

  *word = vr4300->mi_regs[reg];
  debug_mmio_read(mi, mi_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the MI MMIO register space.
int write_mi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct vr4300 *vr4300 = (struct vr4300 *) opaque;
  uint32_t offset = address - MI_REGS_BASE_ADDRESS;
  enum mi_register reg = (offset >> 2);

  debug_mmio_write(mi, mi_register_mnemonics[reg], word, dqm);
  vr4300->mi_regs[reg] &= ~dqm;
  vr4300->mi_regs[reg] |= word;
  return 0;
}

