//
// os/x11/gl_context.h: X11 GL context definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_X11_GL_CONTEXT
#define CEN64_OS_X11_GL_CONTEXT
#include "gl_common.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "gl_window.h"
#include <stddef.h>

#define CEN64_GL_CONTEXT_BAD (NULL)
typedef GLXContext cen64_gl_context;

// Creates a cen64_gl_context and binds it to the cen64_gl_window.
static inline cen64_gl_context cen64_gl_context_create(cen64_gl_window window) {
  cen64_gl_context c;

  if ((c = glXCreateContext(window->display, window->visual_info,
    NULL, True)) == CEN64_GL_CONTEXT_BAD)
    return CEN64_GL_CONTEXT_BAD;

  if (glXMakeCurrent(window->display, window->window, c) != True) {
    glXDestroyContext(window->display, c);
    c = CEN64_GL_CONTEXT_BAD;
  }

  return c;
}

// Unbinds the cen64_gl_context from the window and releases the context.
static inline void cen64_gl_context_destroy(
  cen64_gl_context context, cen64_gl_window window) {
  glXMakeCurrent(window->display, None, NULL);
  glXDestroyContext(window->display, context);
}

#endif

