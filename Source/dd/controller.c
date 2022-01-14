//
// dd/controller.c: DD controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Mike Ryan
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

//
// Thanks go out to OzOnE and Luigiblood (Seru-kun) for reverse
// engineering, documenting, and assisting with the reversal of
// this device!
//
// Thanks also go out to Happy_ for a clean implementation in MESS
// and for assistance debugging this rat's nest
//

#include "common.h"
#include "local_time.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "dd/controller.h"
#include "ri/controller.h"
#include "vr4300/interface.h"

#ifdef DEBUG_MMIO_REGISTER_ACCESS
#define debug_mmio_read(what, mnemonic, val) printf(#what": READ [%s]: 0x%.8X\n", mnemonic, val)
#define debug_mmio_write(what, mnemonic, val, dqm) printf(#what": WRITE [%s]: 0x%.8X/0x%.8X\n", mnemonic, val, dqm)

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
#define DD_BM_STATUS_RUNNING  0x80000000U
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

#define DD_TRACK_LOCK         0x60000000U

#define DD_DS_BUFFER_ADDRESS  0x05000400
#define DD_C2S_BUFFER_ADDRESS 0x05000000

// Magic numbers courtesy of Happy_
const uint32_t zone_sec_size[16] = {
  232, 216, 208, 192, 176, 160, 144, 128,
  216, 208, 192, 176, 160, 144, 128, 112,
};
#define SECTORS_PER_BLOCK 85
#define BLOCKS_PER_TRACK   2

static void dd_update_bm(struct dd_controller *dd);
static void dd_write_sector(struct dd_controller *dd);
static void dd_read_sector(struct dd_controller *dd);
static void set_offset(struct dd_controller *dd);

// Initializes the DD.
int dd_init(struct dd_controller *dd, struct bus_controller *bus,
  const uint8_t *ddipl, const uint8_t *ddrom, size_t ddrom_size) {
  dd->bus = bus;
  dd->ipl_rom = ddipl;
  dd->rom = ddrom;
  dd->rom_size = ddrom_size;

  dd->retail = true;
  dd->regs[DD_ASIC_ID_REG] = 0x00030000;

  dd->rtc_offset_seconds = 0;

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
  // FIXME this mmio_read won't catch updates to DD_ASIC_CMD_STATUS

  switch (reg) {
    case DD_ASIC_CMD_STATUS:
      if (dd->rom != NULL)
        dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_DISK_PRES;
      else
        dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_DISK_PRES;
      *word = dd->regs[DD_ASIC_CMD_STATUS];

      // important! we do not return this updated status as part of the read
      if ((dd->regs[DD_ASIC_CMD_STATUS] & DD_STATUS_BM_INT) &&
          (SECTORS_PER_BLOCK < dd->regs[DD_ASIC_CUR_SECTOR])) {
        // debug("DD Read Gap, Clearing INT\n");
        dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_BM_INT;
        clear_dd_interrupt(dd->bus->vr4300);
        dd_update_bm(dd);
      }
      break;

    default:
      break;
  }

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

    switch (word) {
      // set time
      case DD_CMD_SET_YEAR_MONTH:
      case DD_CMD_SET_DAY_HOUR:
      case DD_CMD_SET_MIN_SEC: {
        struct time_stamp now;
        get_local_time(&now, dd->rtc_offset_seconds);

        if (word == DD_CMD_SET_YEAR_MONTH) {
          uint8_t year = bcd2byte(dd->regs[DD_ASIC_DATA] >> 24);
          /* 96-99 map to the 1990's, 00-95 map to 2000+ */
          now.year = (year >= 96 ? 0 : 100) + year;
          now.month = bcd2byte(dd->regs[DD_ASIC_DATA] >> 16);
        } else if (word == DD_CMD_SET_DAY_HOUR) {
          now.day = bcd2byte(dd->regs[DD_ASIC_DATA] >> 24);
          now.hour = bcd2byte(dd->regs[DD_ASIC_DATA] >> 16);
        } else if (word == DD_CMD_SET_MIN_SEC) {
          now.min = bcd2byte(dd->regs[DD_ASIC_DATA] >> 24);
          now.sec = bcd2byte(dd->regs[DD_ASIC_DATA] >> 16);
        }

        dd->rtc_offset_seconds = get_offset_seconds(&now);
        break;
      }

      // get time
      case DD_CMD_GET_YEAR_MONTH:
      case DD_CMD_GET_DAY_HOUR:
      case DD_CMD_GET_MIN_SEC: {
        struct time_stamp now;
        get_local_time(&now, dd->rtc_offset_seconds);

        if (word == DD_CMD_GET_YEAR_MONTH)
          dd->regs[DD_ASIC_DATA] = (byte2bcd(now.year) << 24) | (byte2bcd(now.month) << 16);
        else if (word == DD_CMD_GET_DAY_HOUR)
          dd->regs[DD_ASIC_DATA] = (byte2bcd(now.day) << 24) | (byte2bcd(now.hour) << 16);
        else if (word == DD_CMD_GET_MIN_SEC)
          dd->regs[DD_ASIC_DATA] = (byte2bcd(now.min) << 24) | (byte2bcd(now.sec) << 16);
        break;
      }

      case DD_CMD_SEEK_READ:
        dd->regs[DD_ASIC_CUR_TK] = dd->regs[DD_ASIC_DATA] >> 16;
        set_offset(dd);
        dd->regs[DD_ASIC_CUR_TK] |= DD_TRACK_LOCK;
        dd->write = false;
        break;

      case DD_CMD_SEEK_WRITE:
        dd->regs[DD_ASIC_CUR_TK] = dd->regs[DD_ASIC_DATA] >> 16;
        set_offset(dd);
        dd->regs[DD_ASIC_CUR_TK] |= DD_TRACK_LOCK;
        dd->write = true;
        break;

      case DD_CMD_CLR_DSK_CHNG:
        dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_DISK_CHNG;
        break;

      case DD_CMD_CLR_RESET:
        dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_RST_STATE;
        break;

      default:
        debug("Unimplemented DD command %04x\n", word >> 16);
    }

    // Always signal an interrupt in response.
    // debug("Sending MECHA Int\n");
    dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_MECHA_INT;
    signal_dd_interrupt(dd->bus->vr4300);
  }

  // Buffer manager control request: handle it.
  else if (reg == DD_ASIC_BM_STATUS_CTL) {
    uint8_t start_sector = (word >> 16) & 0xff;
    if (start_sector == 0x00) {
      dd->start_block = 0;
      dd->regs[DD_ASIC_CUR_SECTOR] = 0;
    }
    else if (start_sector == 0x5A) {
      dd->start_block = 1;
      dd->regs[DD_ASIC_CUR_SECTOR] = 0;
    } else {
      assert(0 && "sector not aligned");
    }

    if (word & DD_BM_CTL_BLK_TRANS) // start block xfer
      dd->regs[DD_ASIC_BM_STATUS_CTL] |= DD_BM_STATUS_BLOCK;
    if (word & DD_BM_CTL_MECHA_RST) // reset MECHA int
      dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_MECHA_INT;

    // handle reset
    if (word & DD_BM_CTL_RESET)
      dd->bm_reset_held = true;
    if (!(word & DD_BM_CTL_RESET) && dd->bm_reset_held) {
      dd->bm_reset_held = false;
      dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_BM_INT;
      dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_BM_ERR;
      dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_DATA_RQ;
      dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_C2_XFER;
      dd->regs[DD_ASIC_BM_STATUS_CTL] = 0;
      dd->regs[DD_ASIC_CUR_SECTOR] = 0;
      dd->start_block = 0;
      debug("dd: BM RESET\n");
    }

    // clear DD interrupt if BM and MECHA int are clear
    if (!(dd->regs[DD_ASIC_CMD_STATUS] & DD_STATUS_BM_INT) &&
        !(dd->regs[DD_ASIC_CMD_STATUS] & DD_STATUS_MECHA_INT))
      clear_dd_interrupt(dd->bus->vr4300);

    // start transfer
    if (word & DD_BM_CTL_START) {
      if (dd->write && (word & DD_BM_CTL_MNGRMODE))
        debug("Attempt to write disk with BM mode 1\n");
      if (dd->write && !(word & DD_BM_CTL_MNGRMODE))
        debug("Attempt to write disk with BM mode 0\n");
      dd->regs[DD_ASIC_BM_STATUS_CTL] |= DD_BM_STATUS_RUNNING;
      debug("dd: Start BM\n");
      dd_update_bm(dd);
    }
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

// PI write to PI_CART_ADDR_REG
void dd_pi_write(void *opaque, uint32_t address) {
  struct dd_controller *dd = opaque;

  // clear DATA RQ
  if (address == DD_DS_BUFFER_ADDRESS && dd->rom != NULL) {
    dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_DATA_RQ;
    dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_BM_INT;
    clear_dd_interrupt(dd->bus->vr4300);
  }

  // clear C2
  else if (address == DD_C2S_BUFFER_ADDRESS && dd->rom != NULL) {
    dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_C2_XFER;
    dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_BM_INT;
    clear_dd_interrupt(dd->bus->vr4300);
  }
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

  debug_mmio_read(dd, "DD_IPL_ROM", *word);
  return 0;
}

// copy data from RDRAM to DD
int dd_dma_read(void *opaque, uint32_t source, uint32_t dest, uint32_t length) {
  struct dd_controller *dd = (struct dd_controller *)opaque;

  // data buffer
  if (dest == DD_DS_BUFFER_ADDRESS) {
    memcpy(dd->ds_buffer, dd->bus->ri->ram + source, length);
    dd_update_bm(dd);
  }

  // microsequencer, not actually used by CEN
  else if (dest == DD_MS_RAM_ADDRESS) {
    uint32_t offset = dest - DD_MS_RAM_ADDRESS;
    assert(offset + length <= DD_MS_RAM_LEN);
    memcpy(dd->ms_ram + offset, dd->bus->ri->ram + source, length);
  }

  else
    debug("Unknown DD DMA read (%08x)\n", dest);

  return 0;
}

// copy data from DD to RDRAM
int dd_dma_write(void *opaque, uint32_t source, uint32_t dest, uint32_t length) {
  struct dd_controller *dd = (struct dd_controller *)opaque;

  // data buffer
  if (source == DD_DS_BUFFER_ADDRESS) {
    memcpy(dd->bus->ri->ram + dest, dd->ds_buffer, length);
    dd_update_bm(dd);
  }

  // C2 buffer, always 0's
  else if (source == DD_C2S_BUFFER_ADDRESS) {
    memset(dd->bus->ri->ram + dest, 0, length);
    dd_update_bm(dd);
  }

  else
    debug("Unknown DD DMA write (%08x)\n", source);

  return 0;
}

// Writes a word to the DD IPL ROM.
int write_dd_ipl_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  assert(0 && "Attempt to write to DD IPL ROM.");
  return 0;
}

// Reads a word from the DD C2S/DS buffer.
int read_dd_controller(void *unused(opaque), uint32_t address, uint32_t *word) {
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
int write_dd_controller(void *unused(opaque), uint32_t address, uint32_t word, uint32_t dqm) {
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
int read_dd_ms_ram(void *unused(opaque), uint32_t address, uint32_t *word) {
  debug_mmio_read(dd, "DD_MS_RAM", *word);
  return 0;
}

// Writes a word to the DD MS RAM.
int write_dd_ms_ram(void *unused(opaque), uint32_t address, uint32_t word, uint32_t dqm) {
  debug_mmio_write(dd, "DD_MS_RAM", word, dqm);
  return 0;
}

void dd_update_bm(struct dd_controller *dd) {
  // quit if not running
  if (!(dd->regs[DD_ASIC_BM_STATUS_CTL] & DD_BM_STATUS_RUNNING))
    return;

  // handle writes
  if (dd->write) {
    // first sector: issue an interrupt to actually get data to be written
    if (dd->regs[DD_ASIC_CUR_SECTOR] == 0) {
      dd->regs[DD_ASIC_CUR_SECTOR] += 1;
      dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_DATA_RQ;
    }

    // subsequent data sectors, write previous sector's data
    else if (dd->regs[DD_ASIC_CUR_SECTOR] < SECTORS_PER_BLOCK) {
      dd_write_sector(dd);
      dd->regs[DD_ASIC_CUR_SECTOR] += 1;
      dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_DATA_RQ;
    }

    // final data sector: write the last sector
    else if (dd->regs[DD_ASIC_CUR_SECTOR] < SECTORS_PER_BLOCK + 1) {
      // continue to next block
      if (dd->regs[DD_ASIC_BM_STATUS_CTL] & DD_BM_STATUS_BLOCK) {
        dd_write_sector(dd);
        dd->start_block = 1 - dd->start_block;
        dd->regs[DD_ASIC_CUR_SECTOR] = 1;
        dd->regs[DD_ASIC_BM_STATUS_CTL] &= ~DD_BM_STATUS_BLOCK;
        dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_DATA_RQ;
      }
      // quit writing after second block
      else {
        dd_write_sector(dd);
        dd->regs[DD_ASIC_CUR_SECTOR] += 1;
        dd->regs[DD_ASIC_BM_STATUS_CTL] &= ~DD_BM_STATUS_RUNNING;
      }
    } else {
      assert(0 && "DD write sector overrun");
    }
    dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_BM_INT;
    signal_dd_interrupt(dd->bus->vr4300);
  }

  // if we aren't writing, we're doing a read BM mode 1

  // track 6 fails to read on retail units
  if (dd->retail &&
      ((dd->regs[DD_ASIC_CUR_TK] & 0xFFF) == 6) && (dd->start_block == 0)) {
    // fail read LBA 12 if retail drive
    dd->regs[DD_ASIC_CMD_STATUS] &= ~DD_STATUS_DATA_RQ;
    dd->regs[DD_ASIC_BM_STATUS_CTL] |= DD_BM_STATUS_MICRO;
  }

  // data sectors: read into buffer and signal interrupt
  else if (dd->regs[DD_ASIC_CUR_SECTOR] < SECTORS_PER_BLOCK) {
    dd_read_sector(dd);
    dd->regs[DD_ASIC_CUR_SECTOR] += 1;
    dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_DATA_RQ;
  }

  // C2 sectors: do nothing since they're always loaded with 0's
  else if (dd->regs[DD_ASIC_CUR_SECTOR] < SECTORS_PER_BLOCK + 4) {
    dd->regs[DD_ASIC_CUR_SECTOR] += 1;
    if (dd->regs[DD_ASIC_CUR_SECTOR] == SECTORS_PER_BLOCK + 4)
      dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_C2_XFER;
  }

  // gap sector
  else if (dd->regs[DD_ASIC_CUR_SECTOR] == SECTORS_PER_BLOCK + 4) {
    if (dd->regs[DD_ASIC_BM_STATUS_CTL] & DD_BM_STATUS_BLOCK) {
      dd->start_block = 1 - dd->start_block;
      dd->regs[DD_ASIC_CUR_SECTOR] = 0;
      dd->regs[DD_ASIC_BM_STATUS_CTL] &= ~DD_BM_STATUS_BLOCK;
    } else {
      dd->regs[DD_ASIC_BM_STATUS_CTL] &= ~DD_BM_STATUS_RUNNING;
    }
  } else {
    assert(0 && "DD read sector overrun");
  }

  dd->regs[DD_ASIC_CMD_STATUS] |= DD_STATUS_BM_INT;
  signal_dd_interrupt(dd->bus->vr4300);
}

void dd_write_sector(struct dd_controller *dd) {
  // TODO actually write data
  /*
  uint32_t offset = dd->track_offset +
    dd->start_block * SECTORS_PER_BLOCK * zone_sec_size[dd->zone] +
    dd->regs[DD_ASIC_CUR_SECTOR] * zone_sec_size[dd->zone];

  memcpy(dd->ds_buffer, dd->rom + offset, zone_sec_size[dd->zone]);
  */
}

void dd_read_sector(struct dd_controller *dd) {
  uint32_t offset = dd->track_offset +
    dd->start_block * SECTORS_PER_BLOCK * zone_sec_size[dd->zone] +
    dd->regs[DD_ASIC_CUR_SECTOR] * zone_sec_size[dd->zone];

  memcpy(dd->ds_buffer, dd->rom + offset, zone_sec_size[dd->zone]);
}

// This magic brought to you by Happy_
void set_offset(struct dd_controller *dd) {
  uint16_t head   = (dd->regs[DD_ASIC_CUR_TK] & 0x1000) >> 9;
  uint16_t track  = dd->regs[DD_ASIC_CUR_TK] & 0xFFF;
  uint16_t tr_off = 0;

  static const uint32_t start_offset[16] = {
    0x00000000, 0x005F15E0, 0x00B79D00, 0x010801A0,
    0x01523720, 0x01963D80, 0x01D414C0, 0x020BBCE0,
    0x023196E0, 0x028A1E00, 0x02DF5DC0, 0x03299340,
    0x036D99A0, 0x03AB70E0, 0x03E31900, 0x04149200
  };

  if (track >= 0x425) {
    dd->zone = 7 + head;
    tr_off = track - 0x425;
  } else if (track >= 0x390) {
    dd->zone = 6 + head;
    tr_off = track - 0x390;
  } else if (track >= 0x2FB) {
    dd->zone = 5 + head;
    tr_off = track - 0x2FB;
  } else if (track >= 0x266) {
    dd->zone = 4 + head;
    tr_off = track - 0x266;
  } else if (track >= 0x1D1) {
    dd->zone = 3 + head;
    tr_off = track - 0x1D1;
  } else if (track >= 0x13C) {
    dd->zone = 2 + head;
    tr_off = track - 0x13C;
  } else if (track >= 0x9E) {
    dd->zone = 1 + head;
    tr_off = track - 0x9E;
  } else {
    dd->zone = 0 + head;
    tr_off = track;
  }

  dd->track_offset = start_offset[dd->zone] +
    tr_off * zone_sec_size[dd->zone] * SECTORS_PER_BLOCK * BLOCKS_PER_TRACK;
}

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

const struct dd_variant *dd_identify_variant(struct rom_file *ipl) {
  static struct dd_variant variants[] = {
    {
      "Japanese Retail (v1.2)",
      0xDD,
      { 0xbf, 0x86, 0x19, 0x22, 0xdc, 0xb7, 0x8c, 0x31, 0x63, 0x60,
        0xe3, 0xe7, 0x42, 0xf4, 0xf7, 0x0f, 0xf6, 0x3c, 0x9b, 0xc3, },
    }, {
      "USA Retail Prototype",
      0xDE,
      { 0x3c, 0x5b, 0x93, 0xca, 0x23, 0x15, 0x50, 0xc6, 0x86, 0x93,
        0xa1, 0x4f, 0x03, 0xce, 0xa8, 0xd5, 0xdb, 0xd1, 0xbe, 0x9e, },
    }, {
      "Japanese Dev (v1.0) [DOES NOT BOOT]",
      0xDD,
      { 0x58, 0x67, 0x0c, 0x00, 0x63, 0x79, 0x3a, 0x8f, 0x3b, 0xe9,
        0x57, 0xd7, 0x1d, 0x93, 0x7b, 0x61, 0x88, 0x29, 0xba, 0x9e, },
    }, {
      "Japanese Dev (v1.1)",
      0xDD,
      { 0xb3, 0xe2, 0x6d, 0xbb, 0x4e, 0x94, 0x5f, 0x78, 0xc9, 0x18,
        0xfa, 0xbc, 0x5b, 0x9e, 0x60, 0xfc, 0xf2, 0x62, 0xc4, 0x7b, },
    }, {
      "Japanese TOOL",
      0xDD,
      { 0x10, 0xc4, 0x17, 0x3c, 0x2a, 0x7e, 0xb0, 0x9c, 0x65, 0x79,
        0x81, 0x8f, 0x72, 0xef, 0x18, 0xfa, 0x0b, 0x6d, 0x32, 0xde, },
    },
  };
  static struct dd_variant unknown = { "Unknown", 0xDD, { 0, } };

  unsigned long i;
  uint8_t sha1_calc[20];
  struct dd_variant *variant = NULL;

  if (ipl == NULL || ipl->ptr == NULL)
    return NULL;
  if (ipl->size != 0x400000)
    return &unknown;

  sha1(ipl->ptr, ipl->size, sha1_calc);

  for (i = 0; i < COUNT_OF(variants); ++i) {
    if (memcmp(sha1_calc, variants[i].sha1, SHA1_SIZE) == 0) {
      variant = &variants[i];
      break;
    }
  }

  if (variant == NULL)
    variant = &unknown;

  return variant;
}
