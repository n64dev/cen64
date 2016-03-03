//
// device/cart_db.h: Cart database.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __device_cart_db_h__
#define __device_cart_db_h__

enum cart_db_save_type {
  CART_DB_SAVE_TYPE_EEPROM_4KBIT,
  CART_DB_SAVE_TYPE_EEPROM_16KBIT,
  CART_DB_SAVE_TYPE_FLASH_1MBIT,
  CART_DB_SAVE_TYPE_SRAM_256KBIT,
};

struct cart_db_entry {
  const char *rom_id;
  enum cart_db_save_type save_type;
  const char *description;
};

cen64_cold const struct cart_db_entry *cart_db_get_entry(const uint8_t *rom);
cen64_cold bool cart_db_is_well_formed(void);

#endif

