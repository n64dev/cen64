//
// os/x11/gl_window.h: X11/OpenGL window definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_X11_GL_WINDOW
#define CEN64_OS_X11_GL_WINDOW
#include "gl_common.h"
#include "gl_config.h"
#include "gl_display.h"
#include "gl_screen.h"
#include <stddef.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#define CEN64_GL_WINDOW_BAD (NULL)
struct cen64_gl_window {
  cen64_gl_display display;
  cen64_gl_screen screen;

  Window window;
  Atom wm_delete_window;
  XSetWindowAttributes attr;
  XVisualInfo *visual_info;
};

typedef struct cen64_gl_window *cen64_gl_window;

struct vi_controller;

//
// Creates a (hidden) cen64_gl_window.
//
static inline cen64_gl_window cen64_gl_window_create(
  cen64_gl_display display, cen64_gl_screen screen,
  const cen64_gl_config *config, const char *title) {
  cen64_gl_window window;

  if ((window = malloc(sizeof(*window))) == NULL)
    return CEN64_GL_WINDOW_BAD;

  // Get the visual info for the framebuffer configuration.
  if ((window->visual_info = glXGetVisualFromFBConfig(
    display, *config)) == NULL) {
    free(window);

    return CEN64_GL_WINDOW_BAD;
  }

  // Create a colormap using the visual info.
  window->attr.colormap = XCreateColormap(display, XRootWindow(
    display, screen), window->visual_info->visual, AllocNone);

  // Select the events we'd like to receive, create the window.
  window->attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    ButtonPressMask | StructureNotifyMask;

  window->window = XCreateWindow(display, XRootWindow(display, screen),
    0, 0, 640, 480, 0, window->visual_info->depth, InputOutput,
    window->visual_info->visual, CWBorderPixel | CWColormap | CWEventMask,
    &window->attr);

  // Now that we created the window, setup any atoms/properties we need.
  XSetStandardProperties(display, window->window, title,
    NULL, None, NULL, 0, NULL);

  window->wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window->window, &window->wm_delete_window, 1);

  window->display = display;
  window->screen = screen;
  return window;
}

//
// Releases resources allocated by cen64_gl_window_create.
//
static inline void cen64_gl_window_destroy(cen64_gl_window window) {
  XDestroyWindow(window->display, window->window);
  XFreeColormap(window->display, window->attr.colormap);
  XFree(window->visual_info);

  free(window);
}

// Handles events that come from X11.
bool cen64_gl_window_pump_events(struct vi_controller *vi);

//
// Swaps the front and back buffers of the cen65_gl_window.
//
static inline void cen64_gl_window_swap_buffers(cen64_gl_window window) {
  glXSwapBuffers(window->display, window->window);
}

//
// Unhides the cen64_gl_window.
//
static inline void cen64_gl_window_unhide(cen64_gl_window window) {
  XMapRaised(window->display, window->window);
  XFlush(window->display);
}

#endif

