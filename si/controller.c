//
// si/controller.c: Peripheral interface controller.
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
#include "si/controller.h"

// Initializes the SI.
int si_init(struct si_controller *si,
  struct bus_controller *bus, const uint8_t *rom) {
  si->bus = bus;
  si->rom = rom;

  return 0;
}

// Reads a word from cartridge ROM.
int read_cart_rom(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - ROM_CART_BASE_ADDRESS;
  struct si_controller *si = (struct si_controller*) opaque;

  return 0;
}

// Reads a word from PIF RAM.
int read_pif_ram(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - PIF_RAM_BASE_ADDRESS;
  return 0;
}

// Reads a word from PIF ROM.
int read_pif_rom(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - PIF_ROM_BASE_ADDRESS;
  struct si_controller *si = (struct si_controller*) opaque;

  memcpy(word, si->rom + offset, sizeof(*word));
  return 0;
}

// Reads a word from the SI MMIO register space.
int read_si_regs(void *opaque, uint32_t address, uint32_t *word) {
  unsigned offset = address - SI_REGS_BASE_ADDRESS;
  enum si_register reg = SI_DRAM_ADDR_REG + (offset >> 2);
  struct si_controller *si = (struct si_controller *) opaque;

  *word = si->regs[reg];
  return 0;
}

// Writes a word to cartridge ROM.
int write_cart_rom(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {
  assert("Attempt to write to cart ROM.");

  return -1;
}

// Writes a word to PIF RAM.
int write_pif_ram(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {
  assert("Attempt to write to PIF RAM.");

  return -1;
}

// Writes a word to PIF ROM.
int write_pif_rom(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {
  assert("Attempt to write to PIF ROM.");

  return -1;
}

// Writes a word to the SI MMIO register space.
int write_si_regs(void unused(*opaque),
  uint32_t unused(address), uint32_t unused(*word)) {

  return 0;
}

