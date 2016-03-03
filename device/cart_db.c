//
// device/cart_db.c: Save list detection.
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
  {"CFZ", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "F-Zero X (NTSC)"},
  {"CLB", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Party (NTSC)"},
  {"CZL", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Legend of Zelda: Ocarina of Time (NTSC)"},
  {"NAB", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Air Boarder 64"},
  {"NAD", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Worms Armageddon (USA)"},
  {"NAG", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "AeroGauge"},
  {"NAL", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Super Smash Bros"},
  {"NB7", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Banjo-Tooie"},
  {"NBC", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Blast Corps"},
  {"NBD", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bomberman Hero"},
  {"NBH", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Body Harvest"},
  {"NBK", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Banjo-Kazooie"},
  {"NBM", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bomberman 64"},
  {"NBV", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bomberman 64: The Second Attack!"},
  {"NCC", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Command & Conquer"},
  {"NCH", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Chopper Attack"},
  {"NCK", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "NBA Courtside 2"},
  {"NCT", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Chameleon Twist"},
  {"NCU", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Cruis'n USA"},
  {"NCW", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Cruis'n World"},
  {"NDO", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Donkey Kong 64"},
  {"NDU", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Duck Dodgers"},
  {"NDY", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Diddy Kong Racing"},
  {"NEA", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "PGA European Tour"},
  {"NEP", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Star Wars Episode I: Racer"},
  {"NER", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "AeroFighters Assault (USA)"},
  {"NF2", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "F-1 World Grand Prix II"},
  {"NFH", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bass Hunter 64"},
  {"NFU", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Conker's Bad Fur Day"},
  {"NFW", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "F-1 World Grand Prix"},
  {"NFX", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Fox 64"},
  {"NFZ", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "F-Zero X (PAL)"},
  {"NGC", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "GT 64: Championship Edition"},
  {"NGE", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "007: Goldeneye"},
  {"NGV", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Glover"},
  {"NIC", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Indy Racing 2000"},
  {"NIJ", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Indiana Jones and the Infernal Machine"},
  {"NJF", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Jet Force Gemini"},
  {"NJM", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Earthworm Jim 3D"},
  {"NK2", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Snowboard Kids 2"},
  {"NK4", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Kirby 64: The Crystal Shards"},
  {"NKA", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Fighters Destiny"},
  {"NKG", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "MLB featuring Ken Griffey Jr."},
  {"NKI", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Killer Instinct Gold"},
  {"NKJ", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Ken Griffey Jr.'s Slugfest"},
  {"NKT", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Kart 64"},
  {"NLB", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Party (PAL)"},
  {"NLR", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Lode Runner 3D"},
  {"NM6", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Mega Man 64"},
  {"NM8", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Mario Tennis"},
  {"NMF", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Mario Golf"},
  {"NMI", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mission: Impossible"},
  {"NML", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mickey's Speedway USA"},
  {"NMO", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Monopoly"},
  {"NMQ", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Paper Mario"},
  {"NMR", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Multi Racing Championship"},
  {"NMU", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Big Mountain 2000"},
  {"NMV", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Mario Party 3"},
  {"NMW", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Party 2"},
  {"NMX", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Excitebike 64"},
  {"NN6", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Dr. Mario 64"},
  {"NNA", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Wars Episode I: Battle for Naboo"},
  {"NNB", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "NBA Courtside"},
  {"NOB", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Ogre Battle 64 "},
  {"NP3", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Stadium 2"},
  {"NPD", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Perfect Dark"},
  {"NPF", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Snap"},
  {"NPG", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Hey You, Pikachu!"},
  {"NPN", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Puzzle League"},
  {"NPO", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Stadium"},
  {"NPW", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Pilotwings 64"},
  {"NRC", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Top Gear Overdrive"},
  {"NRE", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Resident Evil 2"},
  {"NRI", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "The New Tetris"},
  {"NRS", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Wars: Rogue Squadron"},
  {"NRZ", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Ridge Racer 64"},
  {"NS6", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Soldier: Vanishing Earth"},
  {"NSA", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "AeroFighters Assault (PAL, Japan)"},
  {"NSC", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Starshot: Space Circus Fever"},
  {"NSM", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Super Mario 64"},
  {"NSQ", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "StarCraft 64"},
  {"NSU", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Rocket: Robot on Wheels"},
  {"NSV", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "SpaceStation Silicon Valley "},
  {"NSW", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Wars: Shadows of the Empire"},
  {"NT9", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Tigger's Honey Hunt"},
  {"NTE", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "1080 Snowboarding"},
  {"NTJ", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Tom and Jerry in Fists of Furry"},
  {"NTM", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mischief Makers"},
  {"NTN", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "All-Star Tennis 99"},
  {"NTP", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Tetrisphere"},
  {"NTR", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Top Gear Rally (PAL, Japan)"},
  {"NTX", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Taz Express"},
  {"NVL", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "V-Rally 99"},
  {"NW2", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "WCW/nWo Revenge"},
  {"NW4", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "WWF No Mercy"},
  {"NWL", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Waialae Country Club"},
  {"NWR", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Wave Race 64"},
  {"NWU", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Worms Armageddon (PAL)"},
  {"NWX", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "WWF WrestleMania 2000"},
  {"NXO", CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Cruis'n Exotica"},
  {"NYS", CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Yoshi's Story"},
  {"NYW", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Harvest Moon 64"},
  {"NZL", CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Legend of Zelda: Ocarina of Time (PAL)"},
  {"NZS", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Legend of Zelda: Majora's Mask (PAL)"},
};

static int cart_db_table_sorter(const void *a, const void *b) {
  const struct cart_db_entry *entry_a = (const struct cart_db_entry *) a;
  const struct cart_db_entry *entry_b = (const struct cart_db_entry *) b;
  return strcmp(entry_a->rom_id, entry_b->rom_id);
}

const struct cart_db_entry *cart_db_get_entry(const uint8_t *rom) {
  char rom_id[4];
  size_t i;

  memcpy(rom_id, rom + 0x3b, sizeof(rom_id) - 1);
  rom_id[3] = '\0';

  for (i = 0; i < NUM_CART_DB_ENTRIES; i++) {
    if (!strcmp(cart_db_table[i].rom_id, rom_id))
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
    if (!strcmp(cart_db_table_sorted[i].rom_id,
      cart_db_table_sorted[i+1].rom_id)) {
      return false;
    }
  }

  for (i = 0; i < NUM_CART_DB_ENTRIES; i++) {
    if (strlen(cart_db_table_sorted[i].rom_id) != 3)
      return false;
  }

  return true;
}

