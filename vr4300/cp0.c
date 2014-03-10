//
// vr4300/cp0.c: VR4300 system control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/cp0.h"
#include "vr4300/cpu.h"

// Initializes the coprocessor.
void vr4300_cp0_init(struct vr4300 unused(*vr4300)) {
  memset(vr4300->regs, 0, sizeof(*vr4300->regs) * NUM_VR4300_CP0_REGISTERS);
}

