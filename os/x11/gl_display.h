//
// os/x11/gl_display.h: X11 display definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_X11_GL_DISPLAY
#define CEN64_OS_X11_GL_DISPLAY
#include "gl_common.h"
#include <stddef.h>
#include <X11/Xlib.h>

#define CEN64_GL_DISPLAY_BAD (NULL)
typedef Display *cen64_gl_display;

//
// Creates a cen64_gl_display (where 'source' selects the display to use).
//
// If source is NULL, it is treated as a don't care.
//
static inline cen64_gl_display cen64_gl_display_create(const char *source) {
  return XOpenDisplay(source);
}

// Releases resources allocated by cen64_gl_display_create.
static inline void cen64_gl_display_destroy(cen64_gl_display display) {
  XCloseDisplay(display);
}

// Returns the number of screens present on a given cen64_gl_display.
static inline int cen64_gl_display_get_num_screens(cen64_gl_display display) {
  return XScreenCount(display);
}

#endif

