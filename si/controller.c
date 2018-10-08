//
// si/controller.c: Serial interface controller.
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
#include "gl_window.h"
#include "ri/controller.h"
#include "si/cic.h"
#include "si/controller.h"
#include "si/rtc.h"
#include "thread.h"
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

static int read_pif_ram(void *opaque, uint32_t address, uint32_t *word);
static int write_pif_ram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

static void pif_process(struct si_controller *si);
static int pif_perform_command(struct si_controller *si, unsigned channel,
  uint8_t *send_buf, uint8_t send_bytes, uint8_t *recv_buf, uint8_t recv_bytes);

static int eeprom_read(struct eeprom *eeprom, uint8_t *send_buf, uint8_t send_bytes, uint8_t *recv_buf, uint8_t recv_bytes);
static int eeprom_write(struct eeprom *eeprom, uint8_t *send_buf, uint8_t send_bytes, uint8_t *recv_buf, uint8_t recv_bytes);

// Initializes the SI.
int si_init(struct si_controller *si, struct bus_controller *bus,
  const uint8_t *pif_rom, const uint8_t *cart_rom,
  const struct dd_variant *dd_variant,
  uint8_t *eeprom, size_t eeprom_size,
  const struct controller *controller) {
  uint32_t cic_seed;

  si->bus = bus;
  si->rom = pif_rom;

  if (cart_rom) {
    if (get_cic_seed(cart_rom, &cic_seed)) {
      printf("Unknown CIC type; is this a byte-swapped ROM?\n");
      return 1;
    }

    si->ram[0x24] = cic_seed >> 24;
    si->ram[0x25] = cic_seed >> 16;
    si->ram[0x26] = cic_seed >>  8;
    si->ram[0x27] = cic_seed >>  0;
  }

  else if (dd_variant != NULL) {
    si->ram[0x24] = 0x00;
    si->ram[0x25] = 0x0A;
    si->ram[0x26] = dd_variant->seed; // 0xDD - JP, 0xDE - US
    si->ram[0x27] = 0x3F;
  }

  // Specify 8MiB RDRAM for 6102/6105 carts.
  if (si->ram[0x26] == 0x3F && si->ram[0x27] == 0x3F)
    bus_write_word(si->bus, 0x318, 0x800000, ~0U);

  else if (si->ram[0x26] == 0x91 && si->ram[0x27] == 0x3F)
    bus_write_word(si->bus, 0x3F0, 0x800000, ~0U);

  // initialize EEPROM
  si->eeprom.data = eeprom;
  si->eeprom.size = eeprom_size;

  // controllers
  memcpy(si->controller, controller, sizeof(struct controller) * 4);

  return 0;
}

// Handles a single PIF command.
int pif_perform_command(struct si_controller *si,
  unsigned channel, uint8_t *send_buf, uint8_t send_bytes,
  uint8_t *recv_buf, uint8_t recv_bytes) {
  uint8_t command = send_buf[0];
  struct bus_controller *bus;

  switch(command) {
    // Read status/reset.
    case 0x00:
    case 0xFF:
      switch(channel) {
        case 0:
          // always return that controller 0 is connected so that users
          // who don't specify a controller on command line will still
          // have good experience
          recv_buf[0] = 0x05;
          recv_buf[1] = 0x00;
          recv_buf[2] = si->controller[channel].pak == PAK_NONE ? 0x00 : 0x01;
          break;

        case 1:
        case 2:
        case 3:
          if (si->controller[channel].present) {
            recv_buf[0] = 0x05;
            recv_buf[1] = 0x00;
            recv_buf[2] = si->controller[channel].pak == PAK_NONE ? 0x00 : 0x01;
          }
          else {
            recv_buf[0] = recv_buf[1] = recv_buf[2] = 0xFF;
            return 1;
          }
          break;

        case 4:
          // XXX hack alert: this returns 16k EEPROM in the case of a
          // 16k EEPROM and returns 4k EEPROM in all other cases. This
          // is likely a hack to make games that expect EEPROM work,
          // even if the user doesn't supply one on the command line.
          recv_buf[0] = 0x00;
          recv_buf[1] = si->eeprom.size == 0x800 ? 0xC0 : 0x80;
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
          memcpy(&bus, si, sizeof(bus));

          if (likely(bus->vi->window)) {
            cen64_mutex_lock(&bus->vi->window->event_mutex);
            memcpy(recv_buf, si->input, sizeof(si->input));
            cen64_mutex_unlock(&bus->vi->window->event_mutex);
          }

          // -nointerface
          else
            memset(recv_buf, 0x0, sizeof(si->input));

          break;

        default:
          return 1;
      }

      break;

    // Read from controller pak
    case 0x02:
      if (channel < 4)
        return controller_pak_read(&si->controller[channel],
            send_buf, send_bytes, recv_buf, recv_bytes);
      else
        assert(0 && "Invalid channel for controller pak read");

    // Write to controller pak
    case 0x03:
      if (channel < 4)
        return controller_pak_write(&si->controller[channel],
            send_buf, send_bytes, recv_buf, recv_bytes);
      else
        assert(0 && "Invalid channel for controller pak write");

    // EEPROM read
    case 0x04:
      if (channel != 4)
        assert(0 && "Invalid channel for EEPROM read");
      return eeprom_read(&si->eeprom, send_buf, send_bytes, recv_buf, recv_bytes);

    // EEPROM write
    case 0x05:
      if (channel != 4)
        assert(0 && "Invalid channel for EEPROM write");
      return eeprom_write(&si->eeprom, send_buf, send_bytes, recv_buf, recv_bytes);

    // RTC status
    case 0x06:
      if (channel != 4)
        assert(0 && "Invalid channel for RTC status");
      return rtc_status(send_buf, send_bytes, recv_buf, recv_bytes);

    // RTC read
    case 0x07:
      if (channel != 4)
        assert(0 && "Invalid channel for RTC read");
      return rtc_read(send_buf, send_bytes, recv_buf, recv_bytes);

    // RTC write
    case 0x08:
      if (channel != 4)
        assert(0 && "Invalid channel for RTC write");
      return rtc_write(send_buf, send_bytes, recv_buf, recv_bytes);

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

  // clear the PIF's busy flag
  si->ram[0x3F] &= ~0x80;
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
int read_pif_rom_and_ram(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - PIF_ROM_BASE_ADDRESS;
  struct si_controller *si = (struct si_controller*) opaque;

  if (address >= PIF_RAM_BASE_ADDRESS)
    return read_pif_ram(opaque, address, word);

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
int write_pif_rom_and_ram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  if (address >= PIF_RAM_BASE_ADDRESS)
    return write_pif_ram(opaque, address, word, dqm);

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


int eeprom_read(struct eeprom *eeprom, uint8_t *send_buf, uint8_t send_bytes, uint8_t *recv_buf, uint8_t recv_bytes) {
  assert(send_bytes == 2 && recv_bytes == 8);

  uint16_t address = send_buf[1] << 3;

  if (eeprom->data != NULL && address <= eeprom->size - 8) {
    memcpy(recv_buf, &eeprom->data[address], 8);
    return 0;
  }

  return 1;
}

static int eeprom_write(struct eeprom *eeprom, uint8_t *send_buf, uint8_t send_bytes, uint8_t *recv_buf, uint8_t recv_bytes) {
  assert(send_bytes == 10);

  uint16_t address = send_buf[1] << 3;

  if (eeprom->data != NULL && address <= eeprom->size - 8) {
    memcpy(&eeprom->data[address], send_buf + 2, 8);
    return 0;
  }

  return 1;
}
