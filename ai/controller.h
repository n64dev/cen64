//
// ai/controller.h: Audio interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __ai_controller_h__
#define __ai_controller_h__
#include "common.h"

struct bus_controller *bus;

enum ai_register {
#define X(reg) reg,
#include "ai/registers.md"
#undef X
  NUM_AI_REGISTERS
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *ap_register_mnemonics[NUM_AI_REGISTERS];
#endif

struct ai_controller {
  struct bus_controller *bus;
  uint32_t regs[NUM_AI_REGISTERS];
};

int ai_init(struct ai_controller *ai, struct bus_controller *bus);
int read_ai_regs(void *opaque, uint32_t address, uint32_t *word);
int write_ai_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

