//
// vi/controller.h: Video interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vi_controller_h__
#define __vi_controller_h__
#include "common.h"

struct bus_controller *bus;

enum vi_register {
#define X(reg) reg,
#include "vi/registers.md"
#undef X
  NUM_VI_REGISTERS
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *vi_register_mnemonics[NUM_VI_REGISTERS];
#endif

struct vi_controller {
  struct bus_controller *bus;
  uint32_t regs[NUM_VI_REGISTERS];
  uint32_t counter;
};

void vi_cycle(struct vi_controller *vi);
int vi_init(struct vi_controller *vi, struct bus_controller *bus);

int read_vi_regs(void *opaque, uint32_t address, uint32_t *word);
int write_vi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

