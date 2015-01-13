//
// os/unix/x11/glx_window.h
//
// Convenience functions for managing rendering windows.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __unix_x11_glx_window_h__
#define __unix_x11_glx_window_h__
#include <pthread.h>

#include "os/gl_window.h"
#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>

struct glx_window {
  Display *display;
  XVisualInfo *visual_info;
  XF86VidModeModeInfo old_mode;

  Atom wm_delete_message;
  XSetWindowAttributes attr;
  Window window;
  int screen;

  GLXContext context;

  // Locks and whatnot for events.
  pthread_mutex_t event_lock;

  char went_fullscreen;
  bool exit_requested;

  // Locks and whatnot for rendering.
  pthread_mutex_t render_lock;
  pthread_cond_t render_cv;

  unsigned frame_xres, frame_yres, frame_xskip, frame_type;
  uint8_t frame_data[MAX_FRAME_DATA_SIZE];
  bool frame_pending;

  // Event/rendering thread.
  pthread_t thread;
};

cen64_cold bool glx_window_exit_requested(struct glx_window *window);
cen64_cold void glx_window_render_frame(struct glx_window *window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type);

#endif

