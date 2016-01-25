//
// device/save_list.c: Save list detection.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device/cart_db.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define NUM_CART_DB_ENTRIES (sizeof(cart_db_table) / sizeof(*cart_db_table))

static const struct cart_db_entry cart_db_table[] = {
  {"MARIOKART64         ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"THE LEGEND OF ZELDA ", CART_DB_SAVE_TYPE_SRAM_512KBIT}
};

static int cart_db_table_sorter(const void *a, const void *b) {
  const struct cart_db_entry *entry_a = (const struct cart_db_entry *) a;
  const struct cart_db_entry *entry_b = (const struct cart_db_entry *) b;
  return strcmp(entry_a->image_name, entry_b->image_name);
}

const struct cart_db_entry *cart_db_get_entry(const uint8_t *rom) {
  char image_name[21];
  size_t i;

  memcpy(image_name, rom + 0x20, sizeof(image_name) - 1);
  image_name[20] = '\0';

  for (i = 0; i < NUM_CART_DB_ENTRIES; i++) {
    if (!strcmp(cart_db_table[i].image_name, image_name))
      return cart_db_table + i;
  }

  return NULL;
}

bool cart_db_is_well_formed(void) {
  struct cart_db_entry cart_db_table_sorted[NUM_CART_DB_ENTRIES];
  size_t i;

  memcpy(cart_db_table_sorted, cart_db_table, sizeof(cart_db_table));
  qsort(cart_db_table_sorted, NUM_CART_DB_ENTRIES, sizeof(*cart_db_table_sorted),
    cart_db_table_sorter);

  for (i = 0; i < NUM_CART_DB_ENTRIES - 1; i++) {
    if (!strcmp(cart_db_table_sorted[i].image_name,
      cart_db_table_sorted[i+1].image_name)) {
      return true;
    }
  }

  for (i = 0; i < NUM_CART_DB_ENTRIES; i++) {
    if (strlen(cart_db_table_sorted[i].image_name) != 20)
      return false;
  }

  return false;
}

