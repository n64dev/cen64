//
// pi/controller.c: Parallel interface controller.
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
#include "pi/controller.h"

// Initializes the PI.
int pi_init(struct pi_controller *pi, struct bus_controller *bus) {
  pi->bus = bus;

  return 0;
}

// Reads a word from the PI MMIO register space.
int read_pi_regs(void *opaque, uint32_t address, uint32_t *word) {
  unsigned offset = address - PI_REGS_BASE_ADDRESS;
  enum pi_register reg = PI_DRAM_ADDR_REG + (offset >> 2);
  struct pi_controller *pi = (struct pi_controller *) opaque;

  *word = pi->regs[reg];
  return 0;
}

// Writes a word to the PI MMIO register space.
int write_pi_regs(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {
  assert("Attempt to write to cart.");

  return -1;
}

