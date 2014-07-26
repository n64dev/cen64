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

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *ai_register_mnemonics[NUM_AI_REGISTERS] = {
#define X(reg) #reg,
#include "ai/registers.md"
#undef X
};
#endif

// Initializes the AI.
int ai_init(struct ai_controller *ai,
  struct bus_controller *bus) {
  ai->bus = bus;

  return 0;
}

// Reads a word from the AI MMIO register space.
int read_ai_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct ai_controller *ai = (struct ai_controller *) opaque;
  unsigned offset = address - AI_REGS_BASE_ADDRESS;
  enum ai_register reg = (offset >> 2);

  *word = ai->regs[reg];
  debug_mmio_read(ai, ai_register_mnemonics[reg], *word);

  if (reg == AI_STATUS_REG)
    *word = 0x80000001;

  return 0;
}

// Writes a word to the AI MMIO register space.
int write_ai_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct ai_controller *ai = (struct ai_controller *) opaque;
  unsigned offset = address - AI_REGS_BASE_ADDRESS;
  enum ai_register reg = (offset >> 2);

  debug_mmio_write(ai, ai_register_mnemonics[reg], word, dqm);
  ai->regs[reg] &= ~dqm;
  ai->regs[reg] |= word;
  return 0;
}

