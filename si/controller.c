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
#include "os/main.h"
#include "ri/controller.h"
#include "si/cic.h"
#include "si/controller.h"
#include "vi/controller.h"
#include "vr4300/interface.h"
#include <assert.h>

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *si_register_mnemonics[NUM_SI_REGISTERS] = {
#define X(reg) #reg,
#include "si/registers.md"
#undef X
};
#endif

static void pif_process(struct si_controller *si);
static int pif_perform_command(struct si_controller *si, unsigned channel,
  uint8_t *send_buf, uint8_t send_bytes, uint8_t *recv_buf, uint8_t recv_bytes);

// Initializes the SI.
int si_init(struct si_controller *si, struct bus_controller *bus,
  const uint8_t *pif_rom, const uint8_t *cart_rom, bool dd_present) {
  uint32_t cic_seed;

  si->bus = bus;
  si->rom = pif_rom;

  // 64DD present? Use that.
  if (dd_present) {
    si->ram[0x24] = 0x00;
    si->ram[0x25] = 0x0A;
    si->ram[0x26] = 0xDD;
    si->ram[0x27] = 0x3F;
  }

  else if (cart_rom) {
    if (get_cic_seed(cart_rom, &cic_seed)) {
      printf("Unknown CIC type; is this a byte-swapped ROM?\n");
      return 1;
    }

    si->ram[0x24] = cic_seed >> 24;
    si->ram[0x25] = cic_seed >> 16;
    si->ram[0x26] = cic_seed >>  8;
    si->ram[0x27] = cic_seed >>  0;
  }

  // Specify 8MiB RDRAM for 6102/6105 carts.
  if (si->ram[0x26] == 0x3F && si->ram[0x27] == 0x3F)
    bus_write_word(si->bus, 0x318, 0x800000, ~0U);

  else if (si->ram[0x26] == 0x91 && si->ram[0x27] == 0x3F)
    bus_write_word(si->bus, 0x3F0, 0x800000, ~0U);

  return 0;
}

// Handles a single PIF command.
int pif_perform_command(struct si_controller *si,
  unsigned channel, uint8_t *send_buf, uint8_t send_bytes,
  uint8_t *recv_buf, uint8_t recv_bytes) {
  uint8_t command = send_buf[0];

  switch(command) {
    // Read status/reset.
    case 0x00:
    case 0xFF:
      switch(channel) {
        case 0:
          recv_buf[0] = 0x05;
          recv_buf[1] = 0x00;
          recv_buf[2] = 0x01;
          break;

        case 1:
        case 2:
        case 3:
          return 1;

        case 4:
          recv_buf[0] = 0x00;
          recv_buf[1] = 0x80;
          recv_buf[2] = 0x00;
          break;

        default:
          assert(0 && "Invalid channel.");
          return 1;
      }

      break;

    // Read from controller.
    case 0x01:
      switch(channel) {
        case 0:
          os_acquire_input(&si->bus->vi->gl_window);
          memcpy(recv_buf, si->input, sizeof(si->input));
          os_release_input(&si->bus->vi->gl_window);
          break;

        default:
          return 1;
      }

      break;

    // Unimplemented command:
    default:
      return 1;
  }

  return 0;
}

// Emulates the PIF operation.
void pif_process(struct si_controller *si) {
  unsigned channel = 0;
  int ptr = 0;

  if (si->command[0x3F] != 0x1)
    return;

  // Logic ripped from MAME.
  while (ptr < 0x3F) {
    int8_t send_bytes = si->command[ptr++];

    if (send_bytes == -2)
      break;

    if (send_bytes < 0)
      continue;

    if (send_bytes > 0 && (send_bytes & 0xC0) == 0) {
      int8_t recv_bytes = si->command[ptr++];
      uint8_t recv_buf[0x40];
      uint8_t send_buf[0x40];
      int result;

      if (recv_bytes == -2)
        break;

      memcpy(send_buf, si->command + ptr, send_bytes);
      ptr += send_bytes;

      result = pif_perform_command(si, channel,
        send_buf, send_bytes, recv_buf, recv_bytes);

      if (result == 0) {
        memcpy(si->ram + ptr, recv_buf, recv_bytes);
        ptr += recv_bytes;
      }

      else
        si->ram[ptr - 2] |= 0x80;
    }

    channel++;
  }

  si->ram[0x3F] = 0;
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

  else if (reg == SI_PIF_ADDR_RD64B_REG) {
    uint32_t offset = si->regs[SI_DRAM_ADDR_REG] & 0x1FFFFFFF;

    pif_process(si);
    memcpy(si->bus->ri->ram + offset,
      si->ram, sizeof(si->ram));

    signal_rcp_interrupt(si->bus->vr4300, MI_INTR_SI);
    si->regs[SI_STATUS_REG] |= 0x1000;
  }

  else if (reg == SI_PIF_ADDR_WR64B_REG) {
    uint32_t offset = si->regs[SI_DRAM_ADDR_REG] & 0x1FFFFFFF;

    memcpy(si->ram, si->bus->ri->ram + offset, sizeof(si->ram));
    memcpy(si->command, si->ram, sizeof(si->command));

    signal_rcp_interrupt(si->bus->vr4300, MI_INTR_SI);
    si->regs[SI_STATUS_REG] |= 0x1000;
  }

  else {
    si->regs[reg] &= ~dqm;
    si->regs[reg] |= word;
  }

  return 0;
}

