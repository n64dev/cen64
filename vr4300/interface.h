//
// vr4300/interface.h: MI interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_interface_h__
#define __vr4300_interface_h__
#include "common.h"

int read_mi_regs(void *opaque, uint32_t address, uint32_t *word);
int write_mi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

