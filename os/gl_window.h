//
// os/gl_window.h
//
// Convenience functions for managing rendering windows.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_gl_window_h__
#define __os_gl_window_h__

#ifdef _WIN32
#include <windows.h>
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#endif

#include <GL/gl.h>

struct bus_controller;

struct gl_window {
  void *window;

  float viuv[8];
  float quad[8];
};

struct gl_window_hints {
  unsigned width, height;

  char fullscreen;
  char double_buffered;
  char color_bits;
  char alpha_bits;

  char depth_bits;
  char stencil_bits;
  char accum_color_bits;
  char accum_alpha_bits;
  char auxiliary_buffers;
};

/* Default is 800x600, double-buffered; all else is don't care. */
cen64_cold void get_default_gl_window_hints(struct gl_window_hints *hints);

cen64_cold int destroy_gl_window(struct gl_window *window);
cen64_cold int create_gl_window(struct bus_controller *bus,
  struct gl_window *window, const struct gl_window_hints *hints);

int gl_window_thread(struct gl_window *window, struct bus_controller *bus);

cen64_hot int gl_swap_buffers(const struct gl_window *window);
cen64_cold void gl_window_resize_cb(int width, int height);

#endif

