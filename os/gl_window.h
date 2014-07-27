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
void get_default_gl_window_hints(struct gl_window_hints *hints);

int destroy_gl_window(struct gl_window *window);
int create_gl_window(struct gl_window *window,
  const struct gl_window_hints *hints);

int gl_swap_buffers(const struct gl_window *window); 
void os_poll_events(struct bus_controller *bus,
  struct gl_window *gl_window);

#endif

