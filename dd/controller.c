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
int dd_init(struct dd_controller *dd, struct bus_controller *bus,
  const uint8_t *rom) {
  dd->bus = bus;
  dd->rom = rom;

  return 0;
}

// Reads a word from the DD MMIO register space.
int read_dd_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
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

// Reads a word from the DD IPL ROM.
int read_dd_ipl_rom(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - DD_IPL_ROM_ADDRESS;
  struct dd_controller *dd = (struct dd_controller*) opaque;

  if (!dd->rom)
    memset(word, 0, sizeof(word));

  else {
    memcpy(word, dd->rom + offset, sizeof(*word));
    *word = byteswap_32(*word);
  }

  //debug_mmio_read(dd, "DD_IPL_ROM", *word);
  return 0;
}

// Writes a word to the DD IPL ROM.
int write_dd_ipl_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  assert(0 && "Attempt to write to DD IPL ROM.");
  return 0;
}

// Reads a word from the DD C2S buffer.
int read_dd_c2s_buffer(void *opaque, uint32_t address, uint32_t *word) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_C2S_BUFFER_ADDRESS;

  debug_mmio_read(dd, "DD_C2S_BUFFER", *word);
  return 0;
}

// Writes a word to the DD C2S BUFFER.
int write_dd_c2s_buffer(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_C2S_BUFFER_ADDRESS;

  debug_mmio_write(dd, "DD_C2S_BUFFER", word, dqm);
  return 0;
}

// Reads a word from the DD DS buffer.
int read_dd_ds_buffer(void *opaque, uint32_t address, uint32_t *word) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_DS_BUFFER_ADDRESS;

  debug_mmio_read(dd, "DD_DS_BUFFER", *word);
  return 0;
}

// Writes a word to the DD DS BUFFER.
int write_dd_ds_buffer(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_DS_BUFFER_ADDRESS;

  debug_mmio_write(dd, "DD_DS_BUFFER", word, dqm);
  return 0;
}

// Reads a word from the DD MS RAM.
int read_dd_ms_ram(void *opaque, uint32_t address, uint32_t *word) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_MS_RAM_ADDRESS;

  debug_mmio_read(dd, "DD_MS_RAM", *word);
  return 0;
}

// Writes a word to the DD MS RAM.
int write_dd_ms_ram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_MS_RAM_ADDRESS;

  debug_mmio_write(dd, "DD_MS_RAM", word, dqm);
  return 0;
}

