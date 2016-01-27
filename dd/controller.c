//
// dd/controller.c: DD controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

//
// Thanks go out to OzOnE and Luigiblood (Seru-kun) for reverse
// engineering, documenting, and assisting with the reversal of
// this device!
//

//
// TODO: Currently, the DD IPL spams the controller with DD_CMD_NOOP.
// This is normal. Once you signify that a disk is present (using the
// DD_STATUS_DISK_PRES), the DD IPL attempts to start performing seeks.
//

#include "common.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "dd/controller.h"
#include "vr4300/interface.h"

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *dd_register_mnemonics[NUM_DD_REGISTERS] = {
#define X(reg) #reg,
#include "dd/registers.md"
#undef X
};
#endif

static int read_dd_regs(void *opaque, uint32_t address, uint32_t *word);
static int write_dd_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

static int read_dd_ms_ram(void *opaque, uint32_t address, uint32_t *word);
static int write_dd_ms_ram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

// ASIC_CMD_STATUS flags.
#define DD_CMD_NOOP           0x00000000U
#define DD_CMD_SEEK_READ      0x00010001U
#define DD_CMD_SEEK_WRITE     0x00020001U
#define DD_CMD_RECALIBRATE    0x00030001U // ???
#define DD_CMD_SLEEP          0x00040000U
#define DD_CMD_START          0x00050001U
#define DD_CMD_SET_STANDBY    0x00060000U
#define DD_CMD_SET_SLEEP      0x00070000U
#define DD_CMD_CLR_DSK_CHNG   0x00080000U
#define DD_CMD_CLR_RESET      0x00090000U
#define DD_CMD_READ_VERSION   0x000A0000U
#define DD_CMD_SET_DISK_TYPE  0x000B0001U
#define DD_CMD_REQUEST_STATUS 0x000C0000U
#define DD_CMD_STANDBY        0x000D0000U
#define DD_CMD_IDX_LOCK_RETRY 0x000E0000U // ???
#define DD_CMD_SET_YEAR_MONTH 0x000F0000U
#define DD_CMD_SET_DAY_HOUR   0x00100000U
#define DD_CMD_SET_MIN_SEC    0x00110000U
#define DD_CMD_GET_YEAR_MONTH 0x00120000U
#define DD_CMD_GET_DAY_HOUR   0x00130000U
#define DD_CMD_GET_MIN_SEC    0x00140000U
#define DD_CMD_FEATURE_INQ    0x001B0000U

#define DD_STATUS_DATA_RQ     0x40000000U
#define DD_STATUS_C2_XFER     0x10000000U
#define DD_STATUS_BM_ERR      0x08000000U
#define DD_STATUS_BM_INT      0x04000000U
#define DD_STATUS_MECHA_INT   0x02000000U
#define DD_STATUS_DISK_PRES   0x01000000U
#define DD_STATUS_BUSY_STATE  0x00800000U
#define DD_STATUS_RST_STATE   0x00400000U
#define DD_STATUS_MTR_N_SPIN  0x00100000U
#define DD_STATUS_HEAD_RTRCT  0x00080000U
#define DD_STATUS_WR_PR_ERR   0x00040000U
#define DD_STATUS_MECHA_ERR   0x00020000U
#define DD_STATUS_DISK_CHNG   0x00010000U

// ASIC_BM_STATUS_CTL flags.
#define DD_BM_STATUS_RUNNING  0x00000000U
#define DD_BM_STATUS_ERROR    0x04000000U
#define DD_BM_STATUS_MICRO    0x02000000U // ???
#define DD_BM_STATUS_BLOCK    0x01000000U
#define DD_BM_STATUS_C1CRR    0x00800000U
#define DD_BM_STATUS_C1DBL    0x00400000U
#define DD_BM_STATUS_C1SNG    0x00200000U
#define DD_BM_STATUS_C1ERR    0x00010000U // Typo ???

#define DD_BM_CTL_START       0x80000000U
#define DD_BM_CTL_MNGRMODE    0x40000000U
#define DD_BM_CTL_INTMASK     0x20000000U
#define DD_BM_CTL_RESET       0x10000000U
#define DD_BM_CTL_DIS_OR_CHK  0x08000000U // ???
#define DD_BM_CTL_DIS_C1_CRR  0x04000000U
#define DD_BM_CTL_BLK_TRANS   0x02000000U
#define DD_BM_CTL_MECHA_RST   0x01000000U

// Initializes the DD.
int dd_init(struct dd_controller *dd, struct bus_controller *bus,
  const uint8_t *ddipl, const uint8_t *ddrom, size_t ddrom_size) {
  dd->bus = bus;
  dd->ipl_rom = ddipl;
  dd->rom = ddrom;
  dd->rom_size = ddrom_size;

  return 0;
}

// Reads a word from the DD MMIO register space.
int read_dd_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_REGS_BASE_ADDRESS;
  enum dd_register reg = (offset >> 2);

  // XXX: There is some 'extra' space in the DD register MMIO
  // space that gets mapped here in order to make the memory
  // map a little more efficient. It shouldn't impact anything,
  // but be wary.

  *word = dd->regs[reg];
  debug_mmio_read(dd, dd_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the DD MMIO register space.
int write_dd_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_REGS_BASE_ADDRESS;
  enum dd_register reg = (offset >> 2);

  // XXX: There is some 'extra' space in the DD register MMIO
  // space that gets mapped here in order to make the memory
  // map a little more efficient. It shouldn't impact anything,
  // but be wary.

  debug_mmio_write(dd, dd_register_mnemonics[reg], word, dqm);

  // Command register written: do something.
  if (reg == DD_ASIC_CMD_STATUS) {

    // Get time [minute/second]:
    if (word == DD_CMD_GET_MIN_SEC) {
      // dd->regs[DD_ASIC_DATA] = ...
    }

    else if (word == DD_CMD_GET_DAY_HOUR) {
      // dd->regs[DD_ASIC_DATA] = ...
    }

    else if (word == DD_CMD_GET_YEAR_MONTH) {
      // dd->regs[DD_ASIC_DATA] = ...
    }

    else if (word == DD_CMD_CLR_RESET)
      dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_RST_STATE;

    // Always signal an interrupt in response.
    dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_MECHA_INT;
    signal_dd_interrupt(dd->bus->vr4300);
  }

  // Buffer manager control request: handle it.
  else if (reg == DD_ASIC_BM_STATUS_CTL) {
    if (word == DD_BM_CTL_RESET)
      dd->regs[DD_ASIC_BM_STATUS_CTL] &= ~DD_BM_CTL_INTMASK;

    else if (word == DD_BM_CTL_MECHA_RST)
      dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_MECHA_INT;

    clear_dd_interrupt(dd->bus->vr4300);
  }

  // This is done by the IPL and a lot of games. The only word
  // ever know to be written to this register is 0xAAAA0000.
  else if (reg == DD_ASIC_HARD_RESET) {
    assert(word == 0xAAAA0000 && "dd: Hard reset without magic word?");

    dd->regs[DD_ASIC_CMD_STATUS] = DD_STATUS_RST_STATE;
  }

  else {
    dd->regs[reg] &= ~dqm;
    dd->regs[reg] |= word;
  }

  return 0;
}

// Reads a word from the DD IPL ROM.
int read_dd_ipl_rom(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - DD_IPL_ROM_ADDRESS;
  struct dd_controller *dd = (struct dd_controller*) opaque;

  if (!dd->ipl_rom)
    memset(word, 0, sizeof(*word));

  else {
    memcpy(word, dd->ipl_rom + offset, sizeof(*word));
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

// Reads a word from the DD C2S/DS buffer.
int read_dd_controller(void *opaque, uint32_t address, uint32_t *word) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_CONTROLLER_ADDRESS;

  // XXX: Hack to reduce memorymap entries.
  if (address >= DD_MS_RAM_ADDRESS)
    return read_dd_ms_ram(opaque, address, word);

  else if (address >= DD_REGS_BASE_ADDRESS)
    return read_dd_regs(opaque, address, word);

  // XXX: Normal CS2/DS buffer access begins here.
  debug_mmio_read(dd, "DD_C2S/DS_BUFFER", *word);
  return 0;
}

// Writes a word to the DD C2S/DS BUFFER.
int write_dd_controller(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct dd_controller *dd = (struct dd_controller *) opaque;
  unsigned offset = address - DD_CONTROLLER_ADDRESS;

  // XXX: Hack to reduce memorymap entries.
  if (address >= DD_MS_RAM_ADDRESS)
    return write_dd_ms_ram(opaque, address, word, dqm);

  else if (address >= DD_REGS_BASE_ADDRESS)
    return write_dd_regs(opaque, address, word, dqm);

  // XXX: Normal CS2/DS buffer access begins here.
  debug_mmio_write(dd, "DD_C2S/DS_BUFFER", word, dqm);
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

