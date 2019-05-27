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

  {"CFZ", "EJ",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "F-Zero X (NTSC)"},
  {"CLB", "EJ",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Party (NTSC)"},
  {"CP2", "J",        CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Stadium 2 (Japan)"},
  {"CPS", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Pokemon Stadium (Japan)"},
  {"CZL", "EJ",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Legend of Zelda: Ocarina of Time (NTSC)"},
  {"N3D", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Doraemon 3: Nobita no Machi SOS!"},
  {"NA2", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Virtual Pro Wrestling 2"},
  {"NAB", "JP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Air Boarder 64"},
  {"NAD", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Worms Armageddon (USA)"},
  {"NAF", "J",        CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Doubutsu no Mori"},
  {"NAG", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "AeroGauge"},
  {"NAL", "EJPU",     CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Super Smash Bros"},
  {"NB5", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Biohazard 2"},
  {"NB6", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Super B-Daman: Battle Phoenix 64"},
  {"NB7", "EJPU",     CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Banjo-Tooie"},
  {"NBC", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Blast Corps"},
  {"NBD", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bomberman Hero"},
  {"NBH", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Body Harvest"},
  {"NBK", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Banjo-Kazooie"},
  {"NBM", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bomberman 64"},
  {"NBN", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bakuretsu Muteki Bangaioh"},
  {"NBV", "EJ",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bomberman 64: The Second Attack!"},
  {"NCC", "DEP",      CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Command & Conquer"},
  {"NCH", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Chopper Attack"},
  {"NCK", "E",        CART_DB_SAVE_TYPE_FLASH_1MBIT,   "NBA Courtside 2"},
  {"NCR", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Penny Racers"},
  {"NCT", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Chameleon Twist"},
  {"NCU", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Cruis'n USA"},
  {"NCW", "EP",       CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Cruis'n World"},
  {"NCX", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Custom Robo"},
  {"NCZ", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Custom Robo V2"},
  {"ND2", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Doraemon 2: Nobita to Hikari no Shinden"},
  {"ND3", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Akumajou Dracula Mokushiroku"},
  {"ND4", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Akumajou Dracula Mokushiroku Gaiden: Legend of Cornell"},
  {"ND6", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Densha de Go! 64"},
  {"NDA", "J",        CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Derby Stallion 64"},
  {"NDO", "EJP",      CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Donkey Kong 64"},
  {"NDR", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Doraemon: Nobita to 3tsu no Seireiseki"},
  {"NDU", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Duck Dodgers"},
  {"NDY", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Diddy Kong Racing"},
  {"NEA", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "PGA European Tour"},
  {"NEP", "EJP",      CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Star Wars Episode I: Racer"},
  {"NER", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "AeroFighters Assault (USA)"},
  {"NEV", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Neon Genesis Evangelion"},
  {"NF2", "P",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "F-1 World Grand Prix II"},
  {"NFG", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Fighter Destiny 2"},
  {"NFH", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bass Hunter 64"},
  {"NFU", "EP",       CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Conker's Bad Fur Day"},
  {"NFW", "DEFJP",    CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "F-1 World Grand Prix"},
  {"NFX", "EJPU",     CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Fox 64"},
  {"NFZ", "P",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "F-Zero X (PAL)"},
  {"NG6", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Ganbare Goemon: Dero Dero Douchuu Obake Tenkomori"},
  {"NGC", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "GT 64: Championship Edition"},
  {"NGE", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "007: GoldenEye"},
  {"NGL", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Getter Love!!"},
  {"NGP", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Goemon: Mononoke Sugoroku"},
  {"NGU", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Tsumi to Batsu: Hoshi no Keishousha"},
  {"NGV", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Glover"},
  {"NHA", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Bomber Man 64 (Japan)"},
  {"NHF", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "64 Hanafuda: Tenshi no Yakusoku"},
  {"NHP", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Heiwa Pachinko World 64"},
  {"NHY", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Hybrid Heaven (Japan)"},
  {"NIC", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Indy Racing 2000"},
  {"NIJ", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Indiana Jones and the Infernal Machine"},
  {"NIM", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Ide Yosuke no Mahjong Juku"},
  {"NJ5", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Jikkyou Powerful Pro Yakyuu 5"},
  {"NJF", "EJP",      CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Jet Force Gemini"},
  {"NJG", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Jinsei Game 64"},
  {"NJM", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Earthworm Jim 3D"},
  {"NK2", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Snowboard Kids 2"},
  {"NK4", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Kirby 64: The Crystal Shards"},
  {"NKA", "DEFJP",    CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Fighters Destiny"},
  {"NKG", "EP",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "MLB featuring Ken Griffey Jr."},
  {"NKI", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Killer Instinct Gold"},
  {"NKJ", "E",        CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Ken Griffey Jr.'s Slugfest"},
  {"NKT", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Kart 64"},
  {"NLB", "P",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Party (PAL)"},
  {"NLL", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Last Legion UX"},
  {"NLR", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Lode Runner 3D"},
  {"NM6", "E",        CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Mega Man 64"},
  {"NM8", "EJP",      CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Mario Tennis"},
  {"NMF", "EJP",      CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Mario Golf"},
  {"NMI", "DEFIPS",   CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mission: Impossible"},
  {"NML", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mickey's Speedway USA"},
  {"NMO", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Monopoly"},
  {"NMQ", "EJP",      CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Paper Mario"},
  {"NMR", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Multi Racing Championship"},
  {"NMS", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Morita Shougi 64"},
  {"NMU", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Big Mountain 2000"},
  {"NMV", "EJP",      CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Mario Party 3"},
  {"NMW", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mario Party 2"},
  {"NMX", "EJP",      CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Excitebike 64"},
  {"NN6", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Dr. Mario 64"},
  {"NNA", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Wars Episode I: Battle for Naboo"},
  {"NNB", "EP",       CART_DB_SAVE_TYPE_EEPROM_16KBIT, "NBA Courtside"},
  {"NOB", "EJ",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Ogre Battle 64"},
  {"NOH", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Transformers: Beast Wars Transmetals"},
  {"NOS", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "64 Oozumou"},
  {"NP2", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Chou Kuukan Nighter Pro Yakyuu King 2"},
  {"NP3", "DEFIJPS",  CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Stadium 2 (USA, PAL)"},
  {"NP6", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Jikkyou Powerful Pro Yakyuu 6"},
  {"NPA", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Jikkyou Powerful Pro Yakyuu 2000"},
  {"NPD", "EJP",      CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Perfect Dark"},
  {"NPE", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Jikkyou Powerful Pro Yakyuu Basic Ban 2001"},
  {"NPF", "DEFIJPSU", CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Snap"},
  {"NPG", "EJ",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Hey You, Pikachu!"},
  {"NPN", "DEFP",     CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Puzzle League"},
  {"NPO", "DEFIPS",   CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Pokemon Stadium (USA, PAL)"},
  {"NPP", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Parlor! Pro 64"},
  {"NPS", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Jikkyou J.League 1999: Perfect Striker 2"},
  {"NPT", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Puyo Puyon Party"},
  {"NPW", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Pilotwings 64"},
  {"NPY", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Puyo Puyo Sun 64"},
  {"NR7", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Robot Poncots 64: 7tsu no Umi no Caramel"},
  {"NRA", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Rally '99"},
  {"NRC", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Top Gear Overdrive"},
  {"NRE", "EP",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Resident Evil 2"},
  {"NRH", "J",        CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Rockman Dash"},
  {"NRI", "EP",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "The New Tetris"},
  {"NRS", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Wars: Rogue Squadron"},
  {"NRZ", "EP",       CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Ridge Racer 64"},
  {"NS4", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Super Robot Taisen 64"},
  {"NS6", "EJ",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Soldier: Vanishing Earth"},
  {"NSA", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "AeroFighters Assault (PAL, Japan)"},
  {"NSC", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Starshot: Space Circus Fever"},
  {"NSI", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Fushigi no Dungeon: Fuurai no Shiren 2"},
  {"NSM", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Super Mario 64"},
  {"NSN", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Snow Speeder"},
  {"NSQ", "DEP",      CART_DB_SAVE_TYPE_FLASH_1MBIT,   "StarCraft 64"},
  {"NSS", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Super Robot Spirits"},
  {"NSU", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Rocket: Robot on Wheels"},
  {"NSV", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "SpaceStation Silicon Valley "},
  {"NSW", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Star Wars: Shadows of the Empire"},
  {"NT3", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Toukon Road 2"},
  {"NT6", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Tetris 64"},
  {"NT9", "EP",       CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Tigger's Honey Hunt"},
  {"NTB", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Transformers: Beast Wars Metals 64"},
  {"NTC", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "64 Trump Collection"},
  {"NTE", "AP",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "1080 Snowboarding"},
  {"NTJ", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Tom and Jerry in Fists of Furry"},
  {"NTM", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Mischief Makers"},
  {"NTN", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "All-Star Tennis 99"},
  {"NTP", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Tetrisphere"},
  {"NTR", "JP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Top Gear Rally (PAL, Japan)"},
  {"NTW", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "64 de Hakken!! Tamagotchi"},
  {"NTX", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Taz Express"},
  {"NUB", "J",        CART_DB_SAVE_TYPE_EEPROM_16KBIT, "PD Ultraman Battle Collection 64"},
  {"NUM", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Nushi Zuri 64: Shiokaze ni Notte"},
  {"NUT", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Nushi Zuri 64"},
  {"NVL", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "V-Rally 99 (USA, PAL)"},
  {"NVP", "J",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Virtual Pro Wrestling 64"},
  {"NVY", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "V-Rally 99 (Japan)"},
  {"NW2", "EP",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "WCW/nWo Revenge"},
  {"NW4", "EP",       CART_DB_SAVE_TYPE_FLASH_1MBIT,   "WWF No Mercy"},
  {"NWL", "EP",       CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Waialae Country Club"},
  {"NWQ", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Rally Challenge 2000"},
  {"NWR", "EJP",      CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Wave Race 64"},
  {"NWU", "P",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Worms Armageddon (PAL)"},
  {"NWX", "EJP",      CART_DB_SAVE_TYPE_SRAM_256KBIT,  "WWF WrestleMania 2000"},
  {"NXO", "E",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Cruis'n Exotica"},
  {"NYK", "J",        CART_DB_SAVE_TYPE_EEPROM_4KBIT,  "Yakouchuu II: Satsujin Kouro"},
  {"NYS", "EJP",      CART_DB_SAVE_TYPE_EEPROM_16KBIT, "Yoshi's Story"},
  {"NYW", "EJ",       CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Harvest Moon 64"},
  {"NZL", "P",        CART_DB_SAVE_TYPE_SRAM_256KBIT,  "Legend of Zelda: Ocarina of Time (PAL)"},
  {"NZS", "EJP",      CART_DB_SAVE_TYPE_FLASH_1MBIT,   "Legend of Zelda: Majora's Mask"},
};

static int cart_db_table_sorter(const void *a, const void *b) {
  const struct cart_db_entry *entry_a = (const struct cart_db_entry *) a;
  const struct cart_db_entry *entry_b = (const struct cart_db_entry *) b;
  return strcmp(entry_a->rom_id, entry_b->rom_id);
}

const struct cart_db_entry *cart_db_get_entry(const uint8_t *rom) {
  char rom_id[4];
  size_t i;

  memcpy(rom_id, rom + 0x3b, sizeof(rom_id));

  for (i = 0; i < NUM_CART_DB_ENTRIES; i++) {
    if (!strncmp(cart_db_table[i].rom_id, rom_id, 3) &&
        strchr(cart_db_table[i].regions, rom_id[3]))
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

