//
// rdp/interface.c: RDP interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "rdp/cpu.h"
#include "rdp/interface.h"

// Reads a word from the DP MMIO register space.
int read_dp_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct rdp *rdp = (struct rdp *) opaque;
  uint32_t offset = address - DP_REGS_BASE_ADDRESS;
  enum dp_register reg = (offset >> 2);

  *word = rdp->regs[reg];
  debug_mmio_read(dp, dp_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the DP MMIO register space.
int write_dp_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rdp *rdp = (struct rdp *) opaque;
  uint32_t offset = address - DP_REGS_BASE_ADDRESS;
  enum dp_register reg = (offset >> 2);

  debug_mmio_write(dp, dp_register_mnemonics[reg], word, dqm);
  rdp->regs[reg] &= ~dqm;
  rdp->regs[reg] |= word;
  return 0;
}

