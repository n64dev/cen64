//
// ri/controller.c: RAM interface controller.
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
#include "ri/controller.h"

// Initializes the RI.
int ri_init(struct ri_controller *ri,
  struct bus_controller *bus) {
  ri->bus = bus;

  return 0;
}

// Reads a word from the RDRAM MMIO register space.
int read_rdram_regs(void *opaque, uint32_t address, uint32_t *word) {
  unsigned offset = address - RDRAM_REGS_BASE_ADDRESS;
  enum ri_register reg = RDRAM_CONFIG_REG + (offset >> 2);
  struct ri_controller *ri = (struct ri_controller *) opaque;

  *word = ri->rdram_regs[reg];
  return 0;
}

// Reads a word from the RI MMIO register space.
int read_ri_regs(void *opaque, uint32_t address, uint32_t *word) {
  unsigned offset = address - RI_REGS_BASE_ADDRESS;
  enum ri_register reg = RI_MODE_REG + (offset >> 2);
  struct ri_controller *ri = (struct ri_controller *) opaque;

  *word = ri->regs[reg];
  return 0;
}

// Writes a word to the RDRAM MMIO register space.
int write_rdram_regs(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {

  return 0;
}

// Writes a word to the RI MMIO register space.
int write_ri_regs(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {

  return 0;
}

