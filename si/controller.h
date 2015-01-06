//
// si/controller.h: Serial interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __si_controller_h__
#define __si_controller_h__
#include "common.h"

struct bus_controller *bus;

enum si_register {
#define X(reg) reg,
#include "si/registers.md"
#undef X
  NUM_SI_REGISTERS
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *si_register_mnemonics[NUM_SI_REGISTERS];
#endif

struct si_controller {
  struct bus_controller *bus;
  const uint8_t *rom;
  uint8_t command[64];
  uint8_t ram[64];

  uint32_t regs[NUM_SI_REGISTERS];
  uint32_t pif_status;
  uint8_t input[4];
};

cen64_cold int si_init(struct si_controller *si, struct bus_controller *bus,
  const uint8_t *pif_rom, const uint8_t *cart_rom, bool dd_present);

int read_pif_ram(void *opaque, uint32_t address, uint32_t *word);
int read_pif_rom(void *opaque, uint32_t address, uint32_t *word);
int read_si_regs(void *opaque, uint32_t address, uint32_t *word);
int write_pif_ram(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int write_pif_rom(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int write_si_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

