//
// rdp/cpu.h: RDP processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __rdp_cpu_h__
#define __rdp_cpu_h__
#include "common.h"

enum dp_register {
#define X(reg) reg,
#include "rdp/registers.md"
#undef X
  NUM_DP_REGISTERS
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *dp_register_mnemonics[NUM_DP_REGISTERS];
#endif

struct rdp {
  uint32_t regs[NUM_DP_REGISTERS];
  struct bus_controller *bus;
};

cen64_cold int rdp_init(struct rdp *rdp, struct bus_controller *bus);

#endif

