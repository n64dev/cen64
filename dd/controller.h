//
// dd/controller.h: DD controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __dd_controller_h__
#define __dd_controller_h__
#include "common.h"
#include "bus/address.h"

struct bus_controller *bus;

enum dd_register {
#define X(reg) reg,
#include "dd/registers.md"
#undef X
  NUM_DD_REGISTERS
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *dd_register_mnemonics[NUM_DD_REGISTERS];
#endif

struct dd_controller {
  struct bus_controller *bus;
  const uint8_t *ipl_rom;
  const uint8_t *rom;
  size_t rom_size;
  bool retail;

  bool write;
  uint32_t track_offset;
  uint32_t zone;
  uint8_t start_block;
  bool bm_reset_held;

  uint32_t regs[DD_REGS_ADDRESS_LEN / 4];
  uint8_t c2s_buffer[DD_C2S_BUFFER_LEN];
  uint8_t ds_buffer[DD_DS_BUFFER_LEN];
  uint8_t ms_ram[DD_MS_RAM_LEN];
};

cen64_cold int dd_init(struct dd_controller *dd, struct bus_controller *bus,
  const uint8_t *ddipl, const uint8_t *ddrom, size_t ddrom_size);

void dd_pi_write(void *opaque, uint32_t address);

int dd_dma_read(void *opaque, uint32_t source, uint32_t dest, uint32_t length);
int dd_dma_write(void *opaque, uint32_t source, uint32_t dest, uint32_t length);

int read_dd_ipl_rom(void *opaque, uint32_t address, uint32_t *word);
int write_dd_ipl_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

int read_dd_controller(void *opaque, uint32_t address, uint32_t *word);
int write_dd_controller(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

