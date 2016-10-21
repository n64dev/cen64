//
// si/cic.c: PIF CIC security/lock out algorithms.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "si/cic.h"

#define CIC_SEED_NUS_5101 0x0000AC00U
#define CIC_SEED_NUS_6101 0x00063F3FU
#define CIC_SEED_NUS_6102 0x00023F3FU
#define CIC_SEED_NUS_6103 0x0002783FU
#define CIC_SEED_NUS_6105 0x0002913FU
#define CIC_SEED_NUS_6106 0x0002853FU
#define CIC_SEED_NUS_8303 0x0000DD00U

#define CRC_NUS_5101 0x587BD543U
#define CRC_NUS_6101 0x6170A4A1U
#define CRC_NUS_7102 0x009E9EA3U
#define CRC_NUS_6102 0x90BB6CB5U
#define CRC_NUS_6103 0x0B050EE0U
#define CRC_NUS_6105 0x98BC2C86U
#define CRC_NUS_6106 0xACC8580AU
#define CRC_NUS_8303 0x0E018159U

cen64_cold static uint32_t si_crc32(const uint8_t *data, size_t size);

// Determines the CIC seed for a cart, given the ROM data.
int get_cic_seed(const uint8_t *rom_data, uint32_t *cic_seed) {
  uint32_t crc = si_crc32(rom_data + 0x40, 0x1000 - 0x40);
  uint32_t aleck64crc = si_crc32(rom_data + 0x40, 0xC00 - 0x40);

  if (aleck64crc == CRC_NUS_5101) *cic_seed = CIC_SEED_NUS_5101;
  else switch (crc) {
    case CRC_NUS_6101:
    case CRC_NUS_7102:
      *cic_seed = CIC_SEED_NUS_6101;
      break;

    case CRC_NUS_6102:
      *cic_seed = CIC_SEED_NUS_6102;
      break;

    case CRC_NUS_6103:
      *cic_seed = CIC_SEED_NUS_6103;
      break;

    case CRC_NUS_6105:
      *cic_seed = CIC_SEED_NUS_6105;
      break;

    case CRC_NUS_6106:
      *cic_seed = CIC_SEED_NUS_6106;
      break;

    case CRC_NUS_8303:
      *cic_seed = CIC_SEED_NUS_8303;
      break;

    default:
      *cic_seed = 0;
      return 1;
  }

  return 0;
}

uint32_t si_crc32(const uint8_t *data, size_t size) {
  uint32_t table[256];
  unsigned n, k;
  uint32_t c;

  for (n = 0; n < 256; n++) {
    c = (uint32_t) n;

    for (k = 0; k < 8; k++) {
      if (c & 1)
        c = 0xEDB88320L ^ (c >> 1);
      else
        c = c >> 1;
    }

    table[n] = c;
  }

  c = 0L ^ 0xFFFFFFFF;

  for (n = 0; n < size; n++)
    c = table[(c ^ data[n]) & 0xFF] ^ (c >> 8);

  return c ^ 0xFFFFFFFF;
}

