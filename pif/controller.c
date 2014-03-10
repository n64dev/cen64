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
#include "bus/address.h"
#include "bus/controller.h"
#include "pif/controller.h"

// Initializes the PIF.
int init_pif(struct pif_controller *pif,
  struct bus_controller *bus, const uint8_t *rom) {
  pif->bus = bus;
  pif->rom = rom;

  return 0;
}

// Reads a word from PIF ROM.
int read_pif_rom(void *opaque, uint32_t address, uint32_t *word) {
  struct pif_controller *pif = (struct pif_controller*) opaque;
  unsigned offset = address - PIF_ROM_BASE_ADDRESS;

  memcpy(word, pif->rom + offset, sizeof(*word));
  return 0;
}

// Writes a word to PIF ROM.
int write_pif_rom(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {
  assert("Attempt to write to PIF ROM.");

  return -1;
}

