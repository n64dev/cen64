//
// si/controller.c: Peripheral interface controller.
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
#include "si/controller.h"
#include "vr4300/interface.h"
#include <assert.h>

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *si_register_mnemonics[NUM_SI_REGISTERS] = {
#define X(reg) #reg,
#include "si/registers.md"
#undef X
};
#endif

// Initializes the SI.
int si_init(struct si_controller *si,
  struct bus_controller *bus, const uint8_t *rom) {
  si->bus = bus;
  si->rom = rom;

  si->ram[0x26] = 0x3F;
  si->ram[0x27] = 0x3F;
  return 0;
}

// Reads a word from PIF RAM.
int read_pif_ram(void *opaque, uint32_t address, uint32_t *word) {
  struct si_controller *si = (struct si_controller *) opaque;
  unsigned offset = (address - PIF_RAM_BASE_ADDRESS) & 0x3F;

  if (offset == 0x24) {
    si->pif_status = 0x80;

    memcpy(word, si->ram + offset, sizeof(*word));
    *word = byteswap_32(*word);
  }

  else if (offset == 0x3C)
    *word = si->pif_status;

  else {
    memcpy(word, si->ram + offset, sizeof(*word));
    *word = byteswap_32(*word);
  }

  return 0;
}

// Reads a word from PIF ROM.
int read_pif_rom(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - PIF_ROM_BASE_ADDRESS;
  struct si_controller *si = (struct si_controller*) opaque;

  memcpy(word, si->rom + offset, sizeof(*word));
  *word = byteswap_32(*word);
  return 0;
}

// Reads a word from the SI MMIO register space.
int read_si_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct si_controller *si = (struct si_controller *) opaque;
  unsigned offset = address - SI_REGS_BASE_ADDRESS;
  enum si_register reg = (offset >> 2);

  *word = si->regs[reg];
  debug_mmio_read(si, si_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to PIF RAM.
int write_pif_ram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct si_controller *si = (struct si_controller *) opaque;
  unsigned offset = (address - PIF_RAM_BASE_ADDRESS) & 0x3F;
  uint32_t orig_word;

  memcpy(&orig_word, si->ram + offset, sizeof(orig_word));
  orig_word = byteswap_32(orig_word) & ~dqm;
  word = byteswap_32(orig_word | word);
  memcpy(si->ram + offset, &word, sizeof(word));

  si->regs[SI_STATUS_REG] |= 0x1000;
  signal_rcp_interrupt(si->bus->vr4300, MI_INTR_SI);
  return 0;
}

// Writes a word to PIF ROM.
int write_pif_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  assert(0 && "Attempt to write to PIF ROM.");
  return 0;
}

// Writes a word to the SI MMIO register space.
int write_si_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct si_controller *si = (struct si_controller *) opaque;
  unsigned offset = address - SI_REGS_BASE_ADDRESS;
  enum si_register reg = (offset >> 2);

  debug_mmio_write(si, si_register_mnemonics[reg], word, dqm);

  if (reg == SI_STATUS_REG) {
    clear_rcp_interrupt(si->bus->vr4300, MI_INTR_SI);
    si->regs[SI_STATUS_REG] &= ~0x1000;
  }

  else {
    si->regs[reg] &= ~dqm;
    si->regs[reg] |= word;
  }
  return 0;
}

