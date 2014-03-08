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

// Initializes the coprocessor.
void vr4300_cp0_init(struct vr4300_cp0 *cp0) {
  memset(cp0, 0, sizeof(*cp0));
}

