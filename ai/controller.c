//
// ai/controller.c: Audio interface controller.
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
#include "ai/controller.h"

// Initializes the AI.
int ai_init(struct ai_controller *ai,
  struct bus_controller *bus) {
  ai->bus = bus;

  return 0;
}

// Reads a word from the AI MMIO register space.
int read_ai_regs(void *opaque, uint32_t address, uint32_t *word) {
  unsigned offset = address - AI_REGS_BASE_ADDRESS;
  enum ai_register reg = AI_DRAM_ADDR_REG + (offset >> 2);
  struct ai_controller *ai = (struct ai_controller *) opaque;

  *word = ai->regs[reg];
  return 0;
}

// Writes a word to the AI MMIO register space.
int write_ai_regs(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {

  return 0;
}

