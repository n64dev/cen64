//
// os/common/gl_hints.c: OpenGL configuration hints.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "gl_common.h"
#include "gl_hints.h"

const cen64_gl_hints cen64_default_gl_hints = {
  CEN64_GL_CONTEXT_TYPE_RGBA,
  CEN64_GL_DRAWABLE_TYPE_WINDOW,

  1, // double_buffered
  -1, // stereoscopic

  -1, // rgb_color_depth
  -1, // alpha_color_depth
  -1, // depth_buffer_size
  -1, // num_aux_buffers
  -1, // stencil_buffer_size

  -1, // accum_buffer_red_bits
  -1, // accum_buffer_green_bits
  -1, // accum_buffer_blue_bits
  -1, // accum_buffer_alpha_bits
};

