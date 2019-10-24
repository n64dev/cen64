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

enum pi_status {
  PI_STATUS_DMA_BUSY = 1 << 0,
  PI_STATUS_IO_BUSY = 1 << 1,
  PI_STATUS_ERROR = 1 << 2,
  PI_STATUS_INTERRUPT = 1 << 3,
  PI_STATUS_IS_BUSY = PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY
};

enum pi_status_write {
  PI_STATUS_RESET_CONTROLLER = 1 << 0,
  PI_STATUS_CLEAR_INTERRUPT = 1 << 1
};

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
  const struct save_file *sram;
  const struct save_file *flashram_file;
  struct flashram flashram;
  struct is_viewer *is_viewer;

  uint64_t counter;
  uint32_t bytes_to_copy;
  bool is_dma_read;

  uint32_t regs[NUM_PI_REGISTERS];
};

cen64_cold int pi_init(struct pi_controller *pi, struct bus_controller *bus,
  const uint8_t *rom, size_t rom_size, const struct save_file *sram,
  const struct save_file *flashram, struct is_viewer *is);

// Only invoke pi_cycle_ when the counter has expired (timeout).
void pi_cycle_(struct pi_controller *pi);

cen64_flatten cen64_hot static inline void pi_cycle(struct pi_controller *pi) {
  if (unlikely(pi->counter-- == 0))
    pi_cycle_(pi);
}

int read_cart_rom(void *opaque, uint32_t address, uint32_t *word);
int read_pi_regs(void *opaque, uint32_t address, uint32_t *word);
int write_cart_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int write_pi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

int read_flashram(void *opaque, uint32_t address, uint32_t *word);
int write_flashram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int read_sram(void *opaque, uint32_t address, uint32_t *word);
int write_sram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

