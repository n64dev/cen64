//
// si/gb.h: Game Boy pak definitions
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Jason Benaim.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __si_gb_h__
#define __si_gb_h__
#include "pak.h"

struct gb_cart {
  int mbc_type;
  int cartrom_num_banks;
  int reg_rom_bank_low;	// low bits of ROM bank
  int reg_rom_bank_high;  // high bits of ROM bank
  uint8_t* bootrom;
  size_t bootromsize;
  uint8_t* cartrom;
  uint8_t* cartromValid;
  uint8_t* cartrom_bank_zero;
  uint8_t* cartrom_bank_n;
  uint8_t* cartromValid_bank_n;
  uint16_t cart_bank_num;
  size_t cartromsize;
  uint8_t* extram;
  uint8_t* extramValidRead;
  uint8_t* extramValidWrite;
  uint8_t* extram_bank;
  uint8_t* extram_bank_validRead;
  uint8_t* extram_bank_validWrite;
  uint8_t extram_bank_num;
  int extram_enabled;
  int mbc1_mode;
  size_t extram_size;
  int extram_num_banks;
  int battery_backed;
  void (*cleanup)(void);
  char savename[256];
  int huc3_ram_mode;
  FILE *fd;
  int chardev_mode;
};

struct controller;
uint8_t gb_read(struct controller *controller, uint16_t address);
void gb_write(struct controller *controller, uint16_t address, uint8_t data);
void gb_init(struct controller *controller);

#endif
