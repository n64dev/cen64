//
// os/windows/winapi_window.h
//
// Convenience functions for managing rendering windows.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __windows_winapi_window_h__
#define __windows_winapi_window_h__

#include "common.h"
#include <windows.h>
#include <GL/gl.h>

#define MAX_FRAME_DATA_SIZE (640 * 480 * 4)

struct winapi_window {
  HINSTANCE h_instance;
  HWND h_wnd;

  HDC h_dc;
  HGLRC h_glrc;

  // Locks and whatnot for events.
  CRITICAL_SECTION event_lock;

  bool went_fullscreen;
  bool exit_requested;

  // Locks and whatnot for rendering.
  CRITICAL_SECTION render_lock;
  HANDLE render_semaphore;

  unsigned frame_xres, frame_yres, frame_xskip, frame_type;
  uint8_t *frame_data; //[MAX_FRAME_DATA_SIZE];
  bool frame_pending;
};

cen64_cold bool winapi_window_exit_requested(struct winapi_window *window);
cen64_cold void winapi_window_render_frame(struct winapi_window *window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type);

#endif

