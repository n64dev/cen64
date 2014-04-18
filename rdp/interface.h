//
// rdp/interface.h: RDP interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __rdp_interface_h__
#define __rdp_interface_h__
#include "common.h"

int read_dp_regs(void *opaque, uint32_t address, uint32_t *word);
int write_dp_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

