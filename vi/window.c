//
// vi/window.c: Video interface host window/GUI routines.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "gl_common.h"
#include "gl_context.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "gl_window.h"
#include "vi/controller.h"

// Creates a rendering window/context for the VI controller.
int vi_create_window(struct vi_controller *vi) {
  struct cen64_gl_hints hints = cen64_default_gl_hints;
  struct cen64_gl_config *config;
  int num_matching;

  // Create a window for rendering. If we're successful,
  // we'll work our way into the nested statements and
  // return success.
  if ((vi->display = cen64_gl_display_create(
    NULL)) == CEN64_GL_DISPLAY_BAD)
    return -1;

  if ((vi->screen = cen64_gl_screen_create(
    vi->display, -1)) == CEN64_GL_SCREEN_BAD) {
    cen64_gl_display_destroy(vi->display);
    return -1;
  }

  if ((config = cen64_gl_config_create(vi->display, vi->screen,
    &hints, &num_matching)) == CEN64_GL_CONFIG_BAD) {
    cen64_gl_screen_destroy(vi->screen);
    cen64_gl_display_destroy(vi->display);
    return -1;
  }

  vi->window = cen64_gl_window_create(
    vi->display, vi->screen, config, "CEN64");

  cen64_gl_config_destroy(config);

  if (vi->window == CEN64_GL_WINDOW_BAD) {
    cen64_gl_screen_destroy(vi->screen);
    cen64_gl_display_destroy(vi->display);
    return -1;
  }

  if ((vi->context = cen64_gl_context_create(
    vi->window)) == CEN64_GL_CONTEXT_BAD) {
    cen64_gl_window_destroy(vi->window);
    cen64_gl_screen_destroy(vi->screen);
    cen64_gl_display_destroy(vi->display);
    return -1;
  }

  cen64_gl_window_unhide(vi->window);
  return 0;
}

// Destroys the rendering window/context
// associated with the VI controller.
void vi_destroy_window(struct vi_controller *vi) {
  cen64_gl_context_destroy(vi->context, vi->window);
  cen64_gl_window_destroy(vi->window);
  cen64_gl_screen_destroy(vi->screen);
  cen64_gl_display_destroy(vi->display);
}

