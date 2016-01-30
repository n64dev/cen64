//
// pi/controller.h: Parallel interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __pi_controller_h__
#define __pi_controller_h__
#include "common.h"
#include "os/common/save_file.h"

struct bus_controller *bus;

enum pi_register {
#define X(reg) reg,
#include "pi/registers.md"
#undef X
  NUM_PI_REGISTERS
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *pi_register_mnemonics[NUM_PI_REGISTERS];
#endif

#define FLASHRAM_SIZE 0x20000

enum flashram_mode {
  FLASHRAM_IDLE = 0,
  FLASHRAM_ERASE,
  FLASHRAM_WRITE,
  FLASHRAM_READ,
  FLASHRAM_STATUS,
};

struct flashram {
  uint8_t *data;
  uint64_t status;
  enum flashram_mode mode;
  size_t offset;
  size_t rdram_pointer;
};

struct pi_controller {
  struct bus_controller *bus;

  const uint8_t *rom;
  size_t rom_size;
  struct save_file *sram;
  struct save_file *flashram_file;
  struct flashram flashram;

  uint64_t counter;
  uint32_t bytes_to_copy;
  bool is_dma_read;

  uint32_t regs[NUM_PI_REGISTERS];
};

cen64_cold int pi_init(struct pi_controller *pi, struct bus_controller *bus,
  const uint8_t *rom, size_t rom_size, const struct save_file *sram,
  const struct save_file *flashram);

cen64_flatten cen64_hot void ai_cycle(struct ai_controller *ai);

int read_cart_rom(void *opaque, uint32_t address, uint32_t *word);
int read_pi_regs(void *opaque, uint32_t address, uint32_t *word);
int write_cart_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int write_pi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int read_flashram(void *opaque, uint32_t address, uint32_t *word);
int write_flashram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

