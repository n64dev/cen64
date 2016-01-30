//
// pi/controller.c: Parallel interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "dd/controller.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "vr4300/interface.h"
#include <assert.h>

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *pi_register_mnemonics[NUM_PI_REGISTERS] = {
#define X(reg) #reg,
#include "pi/registers.md"
#undef X
};
#endif

static int pi_dma_read(struct pi_controller *pi);
static int pi_dma_write(struct pi_controller *pi);

// Advances the controller by one clock cycle.
void pi_cycle_(struct pi_controller *pi) {

  // DMA engine is finishing up with one entry.
  if (pi->bytes_to_copy > 0) {
    uint32_t bytes = pi->bytes_to_copy;

    // XXX: Defer actual movement of bytes until... now.
    // This is a giant hack; bytes should be DMA'd slowly.
    // Also, why the heck does the OR do? I know the &
    // clears the busy bit, but...
    pi->is_dma_read ? pi_dma_read(pi) : pi_dma_write(pi);

    pi->regs[PI_DRAM_ADDR_REG] += bytes;
    pi->regs[PI_CART_ADDR_REG] += bytes;
    pi->regs[PI_STATUS_REG] &= ~0x1;
    pi->regs[PI_STATUS_REG] |= 0x8;

    signal_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);

    pi->bytes_to_copy = 0;
    return;
  }
}

// Copies data from RDRAM to the PI
static int pi_dma_read(struct pi_controller *pi) {
  uint32_t dest = pi->regs[PI_CART_ADDR_REG] & 0xFFFFFFF;
  uint32_t source = pi->regs[PI_DRAM_ADDR_REG] & 0x7FFFFF;
  uint32_t length = (pi->regs[PI_RD_LEN_REG] & 0xFFFFFF) + 1;

  if (length & 7)
    length = (length + 7) & ~7;

  // SRAM and FlashRAM
  if (dest >= 0x08000000 && dest < 0x08010000) {
    uint32_t addr = dest & 0x00FFFFF;

    // SRAM
    if (pi->sram->ptr != NULL && addr + length <= 0x8000)
      memcpy(pi->sram->ptr + addr, pi->bus->ri->ram + source, length);

    // FlashRAM: Save the RDRAM destination address. Writing happens
    // after the system sends the flash write command (handled in
    // write_flashram)
    else if (pi->flashram.data != NULL && pi->flashram.mode == FLASHRAM_WRITE)
      pi->flashram.rdram_pointer = source;
  }

  return 0;
}

// Copies data from the the PI into RDRAM.
static int pi_dma_write(struct pi_controller *pi) {
  uint32_t dest = pi->regs[PI_DRAM_ADDR_REG] & 0x7FFFFF;
  uint32_t source = pi->regs[PI_CART_ADDR_REG] & 0xFFFFFFF;
  uint32_t length = (pi->regs[PI_WR_LEN_REG] & 0xFFFFFF) + 1;

  if (length & 7)
    length = (length + 7) & ~7;

  if (pi->bus->dd->ipl_rom && (source & 0x06000000) == 0x06000000) {
    source &= 0x003FFFFF;

    if (source + length > 0x003FFFFF)
      length = 0x003FFFFF - source;

    memcpy(pi->bus->ri->ram + dest, pi->bus->dd->ipl_rom + source, length);
  }

  // SRAM and FlashRAM
  else if (source >= 0x08000000 && source < 0x08010000) {
    uint32_t addr = source & 0x00FFFFF;

    if (pi->sram->ptr != NULL && addr + length <= 0x8000)
      memcpy(pi->bus->ri->ram + dest, pi->sram->ptr + addr, length);

    else if (pi->flashram.data != NULL) {
      // SRAM
      if (pi->flashram.mode == FLASHRAM_STATUS) {
        uint64_t status = htonll(pi->flashram.status);
        memcpy(pi->bus->ri->ram + dest, &status, 8);
      }

      // FlashRAM
      else if (pi->flashram.mode == FLASHRAM_READ)
        memcpy(pi->bus->ri->ram + dest, pi->flashram.data + addr * 2, length);
    }
  }

  else if (source >= 0x18000000 && source < 0x18400000) {
    // TODO: 64DD modem
  }

  else if (pi->rom) {
    if (source + length > pi->rom_size) {
      length = pi->rom_size - source;
      //assert(0);
    }

    // TODO: Very hacky.
    if (source < pi->rom_size)
      memcpy(pi->bus->ri->ram + dest, pi->rom + source, length);
  }

  return 0;
}

// Initializes the PI.
int pi_init(struct pi_controller *pi, struct bus_controller *bus,
  const uint8_t *rom, size_t rom_size, const struct save_file *sram,
  const struct save_file *flashram) {
  pi->bus = bus;
  pi->rom = rom;
  pi->rom_size = rom_size;
  pi->sram = sram;
  pi->flashram_file = flashram;
  pi->flashram.data = flashram->ptr;

  pi->bytes_to_copy = 0;
  return 0;
}

// Reads a word from cartridge ROM.
int read_cart_rom(void *opaque, uint32_t address, uint32_t *word) {
  struct pi_controller *pi = (struct pi_controller *) opaque;
  unsigned offset = (address - ROM_CART_BASE_ADDRESS) & ~0x3;

  // TODO: Need to figure out correct behaviour.
  // Should this even happen to begin with?
  if (offset > pi->rom_size) {
    *word = 0;
    return 0;
  }

  memcpy(word, pi->rom + offset, sizeof(*word));
  *word = byteswap_32(*word);
  return 0;
}

// Reads a word from the PI MMIO register space.
int read_pi_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct pi_controller *pi = (struct pi_controller *) opaque;
  unsigned offset = address - PI_REGS_BASE_ADDRESS;
  enum pi_register reg = (offset >> 2);

  *word = pi->regs[reg];

  debug_mmio_read(pi, pi_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to cartridge ROM.
int write_cart_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  //assert(0 && "Attempt to write to cart ROM.");
  return 0;
}

// Writes a word to the PI MMIO register space.
int write_pi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct pi_controller *pi = (struct pi_controller *) opaque;
  unsigned offset = address - PI_REGS_BASE_ADDRESS;
  enum pi_register reg = (offset >> 2);

  debug_mmio_write(pi, pi_register_mnemonics[reg], word, dqm);

  if (reg == PI_STATUS_REG) {
    if (word & 0x1)
      pi->regs[reg] = 0;

    if (word & 0x2) {
      clear_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);
      pi->regs[reg] &= ~0x8;
    }
  }

  else {
    pi->regs[reg] &= ~dqm;
    pi->regs[reg] |= word;

    if (reg == PI_WR_LEN_REG) {
      if (pi->regs[PI_DRAM_ADDR_REG] == 0xFFFFFFFF) {
        pi->regs[PI_STATUS_REG] &= ~0x1;
        pi->regs[PI_STATUS_REG] |= 0x8;

        signal_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);
        return 0;
      }

      pi->bytes_to_copy = (pi->regs[PI_WR_LEN_REG] & 0xFFFFFF) + 1;
      pi->counter = pi->bytes_to_copy / 8 + 1; // Assume ~4 bytes/clock?
      pi->regs[PI_STATUS_REG] |= 0x9; // I'm busy!
      pi->is_dma_read = false;
    }

    else if (reg == PI_RD_LEN_REG) {
      if (pi->regs[PI_DRAM_ADDR_REG] == 0xFFFFFFFF) {
        pi->regs[PI_STATUS_REG] &= ~0x1;
        pi->regs[PI_STATUS_REG] |= 0x8;

        signal_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);
        return 0;
      }

      pi->bytes_to_copy = (pi->regs[PI_RD_LEN_REG] & 0xFFFFFF) + 1;
      pi->counter = pi->bytes_to_copy / 8 + 1; // Assume ~4 bytes/clock?
      pi->regs[PI_STATUS_REG] |= 0x9; // I'm busy!
      pi->is_dma_read = true;
    }
  }

  return 0;
}

// mapped read of flashram
int read_flashram(void *opaque, uint32_t address, uint32_t *word) {
  struct pi_controller *pi = (struct pi_controller *) opaque;
  if (pi->flashram.data == NULL)
    return -1;

  *word = pi->flashram.status >> 32;
  return 0;
}

// mapped write of flashram, commands
int write_flashram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct pi_controller *pi = (struct pi_controller *) opaque;

  if (pi->flashram.data == NULL) {
    debug("write to FlashRAM but no FlashRAM present\n");
    return 1;
  }

  if (address == 0x08000000) {
    debug("write to flash status, ignored");
    return 0;
  }

  switch (word >> 24) {
    case 0x4B: // set erase offset
      pi->flashram.offset = (word & 0xFFFF) * 128;
      break;

    case 0x78: // erase
      pi->flashram.mode = FLASHRAM_ERASE;
      pi->flashram.status = 0x1111800800C20000LL;
      break;

    case 0xA5: // set write offset
      pi->flashram.offset = (word & 0xFFFF) * 128;
      pi->flashram.status = 0x1111800400C20000LL;
      break;

    case 0xB4: // write
      pi->flashram.mode = FLASHRAM_WRITE;
      break;

    case 0xD2: // execute
      // TODO bounds checks
      if (pi->flashram.mode == FLASHRAM_ERASE)
        memset(pi->flashram.data + pi->flashram.offset, 0xFF, 0x80);

      else if (pi->flashram.mode == FLASHRAM_WRITE)
        memcpy(pi->flashram.data + pi->flashram.offset,
            pi->bus->ri->ram + pi->flashram.rdram_pointer, 0x80);

      break;

    case 0xE1: // status
      pi->flashram.mode = FLASHRAM_STATUS;
      pi->flashram.status = 0x1111800100C20000LL;
      break;

    case 0xF0: // read
      pi->flashram.mode = FLASHRAM_READ;
      pi->flashram.status = 0x11118004F0000000LL;
      break;

    default:
      debug("Unknown flashram command %08x\n", word);
  }

  return 0;
}
