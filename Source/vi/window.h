//
// vi/window.h: Video interface host window/GUI routines.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_VI_WINDOW_H
#define CEN64_VI_WINDOW_H

#include "common.h"
#include "vi/controller.h"

cen64_cold int vi_create_window(struct vi_controller *vi);
cen64_cold void vi_destroy_window(struct vi_controller *vi);

#endif

