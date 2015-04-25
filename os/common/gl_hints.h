//
// os/common/gl_hints.h: OpenGL configuration hints.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_COMMON_GL_HINTS
#define CEN64_OS_COMMON_GL_HINTS
#include "gl_common.h"

struct cen64_gl_hints {
  enum cen64_gl_context_type context_type;
  enum cen64_gl_drawable_type drawable_type;
  int double_buffered;
  int stereoscopic;

  // Color depths.
  int rgb_color_depth;
  int alpha_color_depth;

  // Buffer sizes, counts.
  int depth_buffer_size;
  int num_aux_buffers;
  int stencil_buffer_size;

  // Accumulation buffer bits.
  int accum_buffer_red_bits;
  int accum_buffer_green_bits;
  int accum_buffer_blue_bits;
  int accum_buffer_alpha_bits;
};

typedef struct cen64_gl_hints cen64_gl_hints;
extern const cen64_gl_hints cen64_default_gl_hints;

#endif

