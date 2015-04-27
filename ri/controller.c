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

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *rdram_register_mnemonics[NUM_RDRAM_REGISTERS] = {
#define X(reg) #reg,
#include "ri/rdram_registers.md"
#undef X
};
#endif

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *ri_register_mnemonics[NUM_RI_REGISTERS] = {
#define X(reg) #reg,
#include "ri/registers.md"
#undef X
};
#endif

// Initializes the RI.
int ri_init(struct ri_controller *ri, struct bus_controller *bus) {
  ri->bus = bus;

  // MESS uses these, so we will too?
  ri->regs[RI_MODE_REG] = 0xE;
  ri->regs[RI_CONFIG_REG] = 0x40;
  ri->regs[RI_SELECT_REG] = 0x14;
  ri->regs[RI_REFRESH_REG] = 0x63634;

  return 0;
}

// Reads a word from RDRAM.
int read_rdram(void *opaque, uint32_t address, uint32_t *word) {
  struct ri_controller *ri = (struct ri_controller *) opaque;
  unsigned offset = address - RDRAM_BASE_ADDRESS;

  memcpy(word, ri->ram + offset, sizeof(*word));
  *word = byteswap_32(*word);
  return 0;
}

// Reads a word from the RDRAM MMIO register space.
int read_rdram_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct ri_controller *ri = (struct ri_controller *) opaque;
  unsigned offset = address - RDRAM_REGS_BASE_ADDRESS;
  enum rdram_register reg = (offset >> 2);

  *word = ri->rdram_regs[reg];
  debug_mmio_read(rdram, rdram_register_mnemonics[reg], *word);
  return 0;
}

// Reads a word from the RI MMIO register space.
int read_ri_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct ri_controller *ri = (struct ri_controller *) opaque;
  unsigned offset = address - RI_REGS_BASE_ADDRESS;
  enum ri_register reg = (offset >> 2);

  *word = ri->regs[reg];
  debug_mmio_read(ri, ri_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to RDRAM.
int write_rdram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct ri_controller *ri = (struct ri_controller *) opaque;
  unsigned offset = address - RDRAM_BASE_ADDRESS;
  uint32_t orig_word;

  memcpy(&orig_word, ri->ram + offset, sizeof(orig_word));
  orig_word = byteswap_32(orig_word) & ~dqm;
  word = byteswap_32(orig_word | word);
  memcpy(ri->ram + offset, &word, sizeof(word));
  return 0;
}

// Writes a word to the RDRAM MMIO register space.
int write_rdram_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct ri_controller *ri = (struct ri_controller *) opaque;
  unsigned offset = address - RDRAM_REGS_BASE_ADDRESS;
  enum rdram_register reg = (offset >> 2);

  debug_mmio_write(rdram, rdram_register_mnemonics[reg], word, dqm);
  ri->rdram_regs[reg] &= ~dqm;
  ri->rdram_regs[reg] |= word;
  return 0;
}

// Writes a word to the RI MMIO register space.
int write_ri_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct ri_controller *ri = (struct ri_controller *) opaque;
  unsigned offset = address - RI_REGS_BASE_ADDRESS;
  enum ri_register reg = (offset >> 2);

  debug_mmio_write(ri, ri_register_mnemonics[reg], word, dqm);
  ri->regs[reg] &= ~dqm;
  ri->regs[reg] |= word;
  return 0;
}

