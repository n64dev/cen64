//
// pif/controller.c: Peripheral interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/controller.h"
#include "pif/controller.h"

// Initializes the PIF.
int init_pif(struct pif_controller *pif,
  struct bus_controller *bus, const uint8_t *rom) {
  pif->bus = bus;
  pif->rom = rom;

  return 0;
}

// Reads a word from PIFROM.
int read_pifrom(struct pif_controller *pif, uint32_t *word, unsigned off) {
  assert((off & 0x3) == 0 && "read_pifrom: Offset not word aligned.");

  memcpy(word, pif->rom + off, sizeof(*word));
  return 0;
}

