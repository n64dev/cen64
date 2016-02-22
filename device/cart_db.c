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
  {"1080 SNOWBOARDING   ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"AERO FIGHTERS ASSAUL", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"AEROGAUGE           ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"ALL STAR TENNIS '99 ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"BANJO TOOIE         ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"BASS HUNTER 64      ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"BODY HARVEST        ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"BOMBERMAN HERO      ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"BOMBERMAN64U        ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"BOMBERMAN64U2       ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Banjo-Kazooie       ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Battle for Naboo    ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Big Mountain 2000   ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Blast Corps         ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"CHOPPER_ATTACK      ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"CONKER BFD          ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"CRUIS'N WORLD       ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"Chameleon Twist     ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Command&Conquer     ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"Cruis'n USA         ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"CruisnExotica       ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"DONKEY KONG 64      ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"DR.MARIO 64         ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Diddy Kong Racing   ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"EARTHWORM JIM 3D    ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"EXCITEBIKE64        ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"F-ZERO X            ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"F1 WORLD GRAND PRIX ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Fighter's Destiny   ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"GOLDENEYE           ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"GT64                ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Glover              ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"HARVESTMOON64       ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"INDY RACING 2000    ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Indiana Jones       ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"JET FORCE GEMINI    ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"KEN GRIFFEY SLUGFEST", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"KILLER INSTINCT GOLD", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Kirby64             ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"LT DUCK DODGERS     ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Lode Runner 3D      ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MARIOKART64         ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MICKEY USA          ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MISCHIEF MAKERS     ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MISSION IMPOSSIBLE  ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MLB FEATURING K G JR", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"MULTI RACING        ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MarioGolf64         ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"MarioParty          ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MarioParty2         ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"MarioParty3         ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"MarioTennis         ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"Mega Man 64         ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"NBA COURTSIDE       ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"NBA Courtside 2     ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"NEWTETRIS           ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"OgreBattle64        ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"PAPER MARIO         ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"PGA European Tour   ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"POKEMON SNAP        ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"POKEMON STADIUM     ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"POKEMON STADIUM 2   ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"PUZZLE LEAGUE N64   ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"Perfect Dark        ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"Pilot Wings64       ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"RIDGE RACER 64      ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"ROCKETROBOTONWHEELS ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Resident Evil II    ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"Rogue Squadron      ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"SMASH BROTHERS      ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"SNOWBOARD KIDS2     ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"STAR SOLDIER        ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"STAR WARS EP1 RACER ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"STARCRAFT 64        ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"STARFOX64           ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"SUPER MARIO 64      ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Shadow of the Empire", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Silicon Valley      ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Starshot            ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"TETRISPHERE         ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"THE LEGEND OF ZELDA ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"TOM AND JERRY       ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Tigger's Honey Hunt ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"Top Gear Overdrive  ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"V-RALLY             ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"WAVE RACE 64        ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"WCW / nWo  REVENGE  ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"WORMS ARMAGEDDON    ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"WRESTLEMANIA 2000   ", CART_DB_SAVE_TYPE_SRAM_512KBIT},
  {"WWF No Mercy        ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"Waialae Country Club", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"YOSHI STORY         ", CART_DB_SAVE_TYPE_EEPROM_16KBIT},
  {"ZELDA MAJORA'S MASK ", CART_DB_SAVE_TYPE_FLASH_1MBIT},
  {"hey you, pikachu    ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"monopoly            ", CART_DB_SAVE_TYPE_EEPROM_4KBIT},
  {"Killer Instinct Gold", CART_DB_SAVE_TYPE_EEPROM_4KBIT}
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

