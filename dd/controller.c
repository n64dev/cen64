//
// dd/controller.c: DD controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "dd/controller.h"

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *dd_register_mnemonics[NUM_DD_REGISTERS] = {
#define X(reg) #reg,
#include "dd/registers.md"
#undef X
};
#endif

// Initializes the DD.
int dd_init(struct dd_controller *dd, struct bus_controller *bus) {
  dd->bus = bus;

  return 0;
}

// Reads a word from the DD MMIO register space.
int read_dd_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct dd_controller *dd = (struct pi_controller *) opaque;
  unsigned offset = address - DD_REGS_BASE_ADDRESS;
  enum dd_register reg = (offset >> 2);

  *word = dd->regs[reg];
  debug_mmio_read(dd, dd_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the DD MMIO register space.
int write_dd_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_REGS_BASE_ADDRESS;
  enum dd_register reg = (offset >> 2);

  debug_mmio_write(dd, dd_register_mnemonics[reg], word, dqm);

  dd->regs[reg] &= ~dqm;
  dd->regs[reg] |= word;

  return 0;
}

