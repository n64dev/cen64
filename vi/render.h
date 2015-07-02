//
// vi/render.h: Rendering functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_VI_RENDER_H
#define CEN64_VI_RENDER_H

#include "common.h"
#include "vi/controller.h"

cen64_cold void gl_window_init(struct vi_controller *vi);
cen64_cold void gl_window_render_frame(struct vi_controller *vi,
  const uint8_t *buffer, unsigned hres, unsigned vres,
  unsigned hskip, unsigned type);

// Render callbacks.
cen64_cold void gl_window_resize_cb(int width, int height);

#endif

