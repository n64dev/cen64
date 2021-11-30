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
#include "pi/is_viewer.h"
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
    // XXX: Defer actual movement of bytes until... now.
    // This is a giant hack; bytes should be DMA'd slowly.
    pi->is_dma_read ? pi_dma_read(pi) : pi_dma_write(pi);

    pi->regs[PI_STATUS_REG] &= ~PI_STATUS_DMA_BUSY;
    pi->regs[PI_STATUS_REG] |= PI_STATUS_INTERRUPT;

    signal_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);

    pi->bytes_to_copy = 0;
    return;
  }
}

// Copies data from RDRAM to the PI
static int pi_dma_read(struct pi_controller *pi) {
  uint32_t dest = pi->regs[PI_CART_ADDR_REG] & 0xFFFFFFE;
  uint32_t source = pi->regs[PI_DRAM_ADDR_REG] & 0x7FFFFE;
  uint32_t length = (pi->regs[PI_RD_LEN_REG] & 0xFFFFFF) + 1;

  if (source & 0x7)
    length -= source & 0x7;

  // Cartridge Domain 2 Address 2
  if (dest >= 0x08000000 && dest < 0x10000000) {
    // SRAM
    if (pi->sram->ptr != NULL) {
      // SRAM bank selection bits are [19:18]
      uint32_t sram_bank = (dest >> 18) & 3;
      // SRAM bank capacity is 256Kbits (0x8000 bytes)
      uint32_t sram_offset = (sram_bank * 0x8000) + (dest & 0x7FFF);
      // Check SRAM address boundaries to prevent contiguous access across banks
      uint32_t sram_bank_end = (sram_offset + length - 1) / 0x8000;
      if (sram_bank == sram_bank_end && sram_offset + length <= pi->sram->size)
        memcpy((uint8_t *) (pi->sram->ptr) + sram_offset, pi->bus->ri->ram + source, length);
    }
    // FlashRAM: Save the RDRAM destination address. Writing happens
    // after the system sends the flash write command (handled in
    // write_flashram)
    else if (pi->flashram.data != NULL && pi->flashram.mode == FLASHRAM_WRITE)
      pi->flashram.rdram_pointer = source;
  }

  else if ((source & 0x05000000) == 0x05000000)
    dd_dma_read(pi->bus->dd, source, dest, length);

  // FIXME: verify these
  pi->regs[PI_RD_LEN_REG] = 0x7F;
  pi->regs[PI_DRAM_ADDR_REG] = (pi->regs[PI_DRAM_ADDR_REG] + length + 7) & ~7;
  pi->regs[PI_CART_ADDR_REG] = (pi->regs[PI_CART_ADDR_REG] + length + 1) & ~1;

  return 0;
}

static void pi_rom_fetch(struct pi_controller *pi, uint32_t source, int32_t length, uint8_t *dest) {
  int l = length;
  if (source + length > pi->rom_size)
    l = pi->rom_size - source;
  memcpy(dest, pi->rom + source, l);

  // FIXME: verify this on real hardware
  memset(dest+l, 0xFF, length - l);
}

// Copies data from the the PI into RDRAM.
static int pi_dma_write(struct pi_controller *pi) {
  uint32_t dest = pi->regs[PI_DRAM_ADDR_REG] & 0x7FFFFE;
  uint32_t source = pi->regs[PI_CART_ADDR_REG] & 0xFFFFFFE;
  int32_t length = (pi->regs[PI_WR_LEN_REG] & 0xFFFFFF) + 1;

  if (pi->bus->dd->ipl_rom && (source & 0x06000000) == 0x06000000) {
    source &= 0x003FFFFF;

    if (source + length > 0x003FFFFF)
      length = 0x003FFFFF - source;

    memcpy(pi->bus->ri->ram + dest, pi->bus->dd->ipl_rom + source, length);
  }

  else if ((source & 0x05000000) == 0x05000000)
    dd_dma_write(pi->bus->dd, source, dest, length);

  // Cartridge Domain 2 Address 2
  else if (source >= 0x08000000 && source < 0x10000000) {
    // SRAM
    if (pi->sram->ptr != NULL) {
      // SRAM bank selection bits are [19:18]
      uint32_t sram_bank = (source >> 18) & 3;
      // SRAM bank capacity is 256Kbits (0x8000 bytes)
      uint32_t sram_offset = (sram_bank * 0x8000) + (source & 0x7FFF);
      // Check SRAM address boundaries to prevent contiguous access across banks
      uint32_t sram_bank_end = (sram_offset + length - 1) / 0x8000;
      if (sram_bank == sram_bank_end && sram_offset + length <= pi->sram->size)
        memcpy(pi->bus->ri->ram + dest, (const uint8_t *) (pi->sram->ptr) + sram_offset, length);
    }
    // FlashRAM
    else if (pi->flashram.data != NULL) {
      uint32_t flashram_offset = source & 0x1FFFF;
      // FlashRAM status
      if (pi->flashram.mode == FLASHRAM_STATUS) {
        uint64_t status = htonll(pi->flashram.status);
        memcpy(pi->bus->ri->ram + dest, &status, 8);
      }
      // FlashRAM read
      else if (pi->flashram.mode == FLASHRAM_READ)
        memcpy(pi->bus->ri->ram + dest, pi->flashram.data + flashram_offset * 2, length);
    }
  }

  else if (source >= 0x18000000 && source < 0x18400000) {
    // TODO: 64DD modem
  }

  else if (pi->rom) {
    // PI_WR_LEN_REG has a weird behavior when read back. It almost always
    // reads as 0x7F, with the only exception of very short transfers (<= 8
    // bytes) where the actual value is affected by the DRAM alignment. This
    // is just for full accuracy, nobody is probably relying on this value.
    pi->regs[PI_WR_LEN_REG] = 0x7F;
    if (length <= 8)
      pi->regs[PI_WR_LEN_REG] -= pi->regs[PI_DRAM_ADDR_REG] & 7;

    // PI DMA has an internal cache of 128 bytes ("a block"). Data is fetched
    // from ROM and then copied to RDRAM. The first block is handled "specially":
    // if the RDRAM address is not a multiple of 8, the block is shorter so
    // that the RDRAM address becomes a multiple of 8 afterwards, and a faster
    // code-path is triggered (possibly, 64-bit transfers to RDRAM).
    // This is visible because this feature is actually broken: there are two
    // bugs lingering, so that in the end Nintendo documented that only
    // 8-bytes aligned transfers were possible.
    uint8_t mem[128];
    bool first_block = true;

    while (length > 0) {
      uint32_t dest = pi->regs[PI_DRAM_ADDR_REG] & 0x7FFFFE;
      int32_t misalign = dest & 0x7;

      int32_t cur_len = length;
      int32_t block_len = 128 - misalign;
      if (cur_len > block_len)
        cur_len = block_len;

      // Decrease length (for next block). After first block, odd sizes
      // are round up.
      length -= cur_len;
      if (length & 1) length += 1;

      // Fetch block from ROM. ROM is always fetched as 16-bit words,
      // so round up the actual transfer.
      uint32_t source = pi->regs[PI_CART_ADDR_REG] & 0xFFFFFFE;
      int32_t rom_fetch_len = (cur_len + 1) & ~1;
      pi_rom_fetch(pi, source, rom_fetch_len, mem);
      pi->regs[PI_CART_ADDR_REG] += rom_fetch_len;

      // Writeback to RDRAM. Here come the lions.
      if (first_block) {
        // HARDWARE BUG #1: in the first block, there's an off-by-one, so the
        // length is actually rounded up to even size just for the last byte.
        // Notice that ROM transfers are rounded up anyway, so this additional
        // byte was already fetched from ROM.
        if (cur_len == block_len-1)
          cur_len++;

        // HARDWARE BUG #2: the length of data written back is decreased by the
        // RDRAM misalignment. This is wrong because cur_len was already
        // clamped to the block length, so this actually ends up leaving a
        // hole in RDRAM of non-transferred data at the end of the first block.
        cur_len -= misalign;
        if (cur_len < 0)
          cur_len = 0;
      }

      memcpy(pi->bus->ri->ram+dest, mem, cur_len);
      pi->regs[PI_DRAM_ADDR_REG] += cur_len;
      pi->regs[PI_DRAM_ADDR_REG] = (pi->regs[PI_DRAM_ADDR_REG] + 7) & ~7;

      first_block = false;
    }
  }

  return 0;
}

// Initializes the PI.
int pi_init(struct pi_controller *pi, struct bus_controller *bus,
  const uint8_t *rom, size_t rom_size, const struct save_file *sram,
  const struct save_file *flashram, struct is_viewer *is_viewer) {
  pi->bus = bus;
  pi->rom = rom;
  pi->rom_size = rom_size;
  pi->sram = sram;
  pi->flashram_file = flashram;
  pi->flashram.data = flashram->ptr;
  pi->is_viewer = is_viewer;

  pi->regs[PI_RD_LEN_REG] = 0x7F;
  pi->regs[PI_WR_LEN_REG] = 0x7F;

  pi->bytes_to_copy = 0;
  return 0;
}

// Reads a word from cartridge ROM.
int read_cart_rom(void *opaque, uint32_t address, uint32_t *word) {
  struct pi_controller *pi = (struct pi_controller *) opaque;
  unsigned offset = (address - ROM_CART_BASE_ADDRESS) & ~0x3;

  if (pi->is_viewer && is_viewer_map(pi->is_viewer, address))
    return read_is_viewer(pi->is_viewer, address, word);

  // TODO: Need to figure out correct behaviour.
  // Should this even happen to begin with?
  if (pi->rom == NULL || offset > (pi->rom_size - sizeof(*word))) {
    *word = (address >> 16) | (address & 0xFFFF0000);
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
  struct pi_controller *pi = (struct pi_controller *) opaque;

  if (pi->is_viewer && is_viewer_map(pi->is_viewer, address))
    return write_is_viewer(pi->is_viewer, address, word, dqm);

  return 0;
}

// Writes a word to the PI MMIO register space.
int write_pi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct pi_controller *pi = (struct pi_controller *) opaque;
  unsigned offset = address - PI_REGS_BASE_ADDRESS;
  enum pi_register reg = (offset >> 2);

  debug_mmio_write(pi, pi_register_mnemonics[reg], word, dqm);

  if (reg == PI_STATUS_REG) {
    if (word & PI_STATUS_RESET_CONTROLLER)
      pi->regs[reg] &= ~(PI_STATUS_IS_BUSY | PI_STATUS_ERROR);

    if (word & PI_STATUS_CLEAR_INTERRUPT) {
      clear_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);
      pi->regs[reg] &= ~PI_STATUS_INTERRUPT;
    }
  }

  else {
    if (pi->regs[PI_STATUS_REG] & PI_STATUS_IS_BUSY) {
      pi->regs[PI_STATUS_REG] |= PI_STATUS_ERROR;
      return 0;
    }

    pi->regs[reg] &= ~dqm;
    pi->regs[reg] |= word;

    if (reg == PI_DRAM_ADDR_REG) {
      pi->regs[reg] &= 0x00FFFFFE;

    } else if (reg == PI_CART_ADDR_REG) {
      pi->regs[reg] &= 0xFFFFFFFE;
      dd_pi_write(pi->bus->dd, word);
    }

    else if (reg == PI_WR_LEN_REG) {
      if (pi->regs[PI_DRAM_ADDR_REG] == 0xFFFFFFFF) {
        pi->regs[PI_STATUS_REG] &= ~PI_STATUS_DMA_BUSY;
        pi->regs[PI_STATUS_REG] |= PI_STATUS_INTERRUPT;

        signal_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);
        return 0;
      }

      pi->bytes_to_copy = (pi->regs[PI_WR_LEN_REG] & 0xFFFFFF) + 1;
      pi->counter = pi->bytes_to_copy / 2 + 100; // Assume ~2 bytes/clock?
      pi->regs[PI_STATUS_REG] |= PI_STATUS_DMA_BUSY; // I'm busy!
      pi->is_dma_read = false;
    }

    else if (reg == PI_RD_LEN_REG) {
      if (pi->regs[PI_DRAM_ADDR_REG] == 0xFFFFFFFF) {
        pi->regs[PI_STATUS_REG] &= ~PI_STATUS_DMA_BUSY;
        pi->regs[PI_STATUS_REG] |= PI_STATUS_INTERRUPT;

        signal_rcp_interrupt(pi->bus->vr4300, MI_INTR_PI);
        return 0;
      }

      pi->bytes_to_copy = (pi->regs[PI_RD_LEN_REG] & 0xFFFFFF) + 1;
      pi->counter = pi->bytes_to_copy / 2 + 100; // Assume ~2 bytes/clock?
      pi->regs[PI_STATUS_REG] |= PI_STATUS_DMA_BUSY; // I'm busy!
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

// Mapped read of SRAM
int read_sram(void *opaque, uint32_t address, uint32_t *word) {
  fprintf(stderr, "SRAM read\n");
  return 0;
}

// Mapped write of SRAM
int write_sram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  fprintf(stderr, "SRAM write\n");
  return 0;
}
