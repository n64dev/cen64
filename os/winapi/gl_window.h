//
// os/winapi/gl_window.h: WinAPI/OpenGL window definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_WINAPI_GL_WINDOW
#define CEN64_OS_WINAPI_GL_WINDOW
#include "common.h"
#include "gl_common.h"
#include "gl_config.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "thread.h"
#include <windows.h>

#define FRAMEBUF_SZ (640 * 480 * 4)
#define CEN64_GL_WINDOW_BAD (NULL)
struct cen64_gl_window {
  HINSTANCE hinstance;
  HWND hwnd;
  HDC hdc;

  DWORD thread_id;
  int pixel_format;

  cen64_mutex render_mutex;
  uint8_t frame_buffer[FRAMEBUF_SZ];
  unsigned frame_hres, frame_vres;
  unsigned frame_hskip, frame_type;

  cen64_mutex event_mutex;
  bool exit_requested;
};

typedef struct cen64_gl_window *cen64_gl_window;

// Forward declaration.
struct cen64_device;

// Creates a (hidden) cen64_gl_window.
cen64_cold cen64_gl_window cen64_gl_window_create(
  cen64_gl_display display, cen64_gl_screen screen,
  const cen64_gl_config *config, const char *title);

// Releases resources allocated by cen64_gl_window_create.
static inline void cen64_gl_window_destroy(cen64_gl_window window) {
  ReleaseDC(window->hwnd, window->hdc);
  DestroyWindow(window->hwnd);
  UnregisterClass("CEN64", window->hinstance);

  cen64_mutex_destroy(&window->render_mutex);
  cen64_mutex_destroy(&window->event_mutex);
  free(window);
}

// Pushes a notification to the UI queue to indicate a frame is ready.
static inline void cen64_gl_window_push_frame(cen64_gl_window window) {
  PostThreadMessage(window->thread_id, WM_USER, 0, 0);
}

// Swaps the front and back buffers of the cen65_gl_window.
static inline void cen64_gl_window_swap_buffers(cen64_gl_window window) {
  SwapBuffers(window->hdc);
}

// Sets the title of the cen64_gl_window.
static inline void cen64_gl_window_set_title(
  cen64_gl_window window, const char *title) {
  SetWindowText(window->hwnd, title);
}

// Thread that controls the user interface, etc.
int cen64_gl_window_thread(struct cen64_device *device);

// Unhides the cen64_gl_window.
static inline void cen64_gl_window_unhide(cen64_gl_window window) {
  ShowWindow(window->hwnd, SW_SHOW);
}

#endif

