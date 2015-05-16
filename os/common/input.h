//
// os/common/input.h: Input device handling functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_COMMON_INPUT
#define CEN64_OS_COMMON_INPUT
#include "common.h"

struct bus_controller;

cen64_cold void keyboard_press_callback(struct bus_controller *bus, unsigned key);
cen64_cold void keyboard_release_callback(struct bus_controller *bus, unsigned key);

#endif

