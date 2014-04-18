//
// vi/controller.c: Video interface controller.
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
#include "vi/controller.h"

// Initializes the VI.
int vi_init(struct vi_controller *vi,
  struct bus_controller *bus) {
  vi->bus = bus;

  return 0;
}

// Reads a word from the VI MMIO register space.
int read_vi_regs(void *opaque, uint32_t address, uint32_t *word) {
  unsigned offset = address - VI_REGS_BASE_ADDRESS;
  enum vi_register reg = VI_STATUS_REG + (offset >> 2);
  struct vi_controller *vi = (struct vi_controller *) opaque;

  *word = vi->regs[reg];
  return 0;
}

// Writes a word to the VI MMIO register space.
int write_vi_regs(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {

  return 0;
}

