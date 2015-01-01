//
// os/windows/winapi_window.c
//
// Convenience functions for managing rendering windows.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "common/debug.h"
#include "device/device.h"
#include "os/gl_window.h"
#include "os/input.h"
#include "os/timer.h"
#include "os/windows/winapi_window.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <tchar.h>
#include <GL/gl.h>

static int create_gl_context(struct winapi_window *winapi_window, HGLRC *h_rc);

static int get_matching_pixel_format(struct winapi_window *winapi_window,
  const struct gl_window_hints *hints);

cen64_hot static int winapi_window_thread(struct gl_window *gl_window,
  struct winapi_window *winapi_window, struct bus_controller *bus);

static void winapi_window_update_window_title(
  struct winapi_window *winapi_window, cen64_time *last_report_time);

// Classname. TODO: Make this unique?
static const LPCSTR CLASSNAME = "CEN64";

// Callback function to handle message sent to the window.
LRESULT CALLBACK WndProc(HWND hWnd,
  UINT message, WPARAM wParam, LPARAM lParam) {
  switch(message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    case WM_KEYDOWN:
      break;

    case WM_KEYUP:
      break;

    case WM_SIZE:
      gl_window_resize_cb(LOWORD(lParam), HIWORD(lParam));
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

// Creates a new rendering context.
int create_gl_context(struct winapi_window *winapi_window, HGLRC *h_rc) {
  if ((*h_rc = wglCreateContext(winapi_window->h_dc)) == NULL) {
    debug("create_gl_context: wglCreateContext returned NULL.");
    return 1;
  }

  return 0;
}

// Creates a new rendering window.
int create_gl_window(struct bus_controller *bus,
  struct gl_window *gl_window, const struct gl_window_hints *hints) {
  struct winapi_window *winapi_window;
  unsigned fullscreen;

  WNDCLASS wc;
  DWORD dw_ex_style;
  DWORD dw_style;
  RECT window_rect;

  fullscreen = hints->fullscreen;

  window_rect.left = 0;
  window_rect.right = hints->width;
  window_rect.top = 0;
  window_rect.bottom = hints->height;

  // Allocate memory for the opaque handle inside thw window.
  if ((gl_window->window = malloc(sizeof(*winapi_window))) == NULL) {
    debug("create_gl_window: Could not allocate enough memory.\n");
    return 1;
  }

  winapi_window = (struct winapi_window *) (gl_window->window);
  memset(winapi_window, 0, sizeof(*winapi_window));

  InitializeCriticalSection(&winapi_window->render_lock);
  InitializeCriticalSection(&winapi_window->event_lock);
  winapi_window->render_semaphore = CreateSemaphore(
    NULL, 0, 0x7FFFFFFFU, NULL);

  winapi_window->h_instance = GetModuleHandle(NULL);
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = (WNDPROC) WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = winapi_window->h_instance;
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = CLASSNAME;

  if (!RegisterClass(&wc)) {
    debug("create_gl_window: Failed to register the window class.\n");
    goto create_out_destroy;
  }

  dw_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
  dw_style = WS_OVERLAPPEDWINDOW;
  AdjustWindowRectEx(&window_rect, dw_style, FALSE, dw_ex_style);

  if (!(winapi_window->h_wnd = CreateWindowEx(dw_ex_style, CLASSNAME,
    "CEN64 ["CEN64_COMPILER" - "CEN64_ARCH_DIR"/"CEN64_ARCH_SUPPORT"]",
    dw_style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0,
    window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
    NULL, NULL, winapi_window->h_instance, NULL))) {
    debug("create_gl_window: Failed to create a window.\n");
    goto create_out_destroy;
  }

  if ((winapi_window->h_dc = GetDC(winapi_window->h_wnd)) == NULL) {
    debug("create_gl_window: Failed get an GL device context.\n");
    goto create_out_destroy;
  }

  if (get_matching_pixel_format(winapi_window, hints)) {
    debug("create_gl_window: Failed to match GL window hints.\n");
    goto create_out_destroy;
  }

  if (create_gl_context(winapi_window, &winapi_window->h_glrc)) {
    debug("create_gl_window: Failed to acquire a GL rendering context.\n");
    goto create_out_destroy;
  }

  ShowWindow(winapi_window->h_wnd, SW_SHOW);
  SetForegroundWindow(winapi_window->h_wnd);
  SetFocus(winapi_window->h_wnd);

  // All ready; kickoff the thread.
  if ((winapi_window->frame_data = malloc(MAX_FRAME_DATA_SIZE)) == NULL) {
    debug("create_gl_window: Failed to allocate memory for frame data.\n");

    goto create_out_destroy;
  }

  return 0;

create_out_destroy:
  destroy_gl_window(gl_window);
  return 1;
}

// Destroys an existing rendering window.
int destroy_gl_window(struct gl_window *window) {
  struct winapi_window *winapi_window;

  winapi_window = (struct winapi_window *) (window->window);

  if (winapi_window->h_glrc) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(winapi_window->h_glrc);
    winapi_window->h_glrc = NULL;
  }

  if (winapi_window->h_dc) {
    ReleaseDC(winapi_window->h_wnd, winapi_window->h_dc);
    winapi_window->h_dc = NULL;
  }

  if (winapi_window->h_wnd) {
    DestroyWindow(winapi_window->h_wnd);
    winapi_window->h_wnd = NULL;
  }

  if (winapi_window->h_instance) {
    UnregisterClass(CLASSNAME, winapi_window->h_instance);
    winapi_window->h_instance = NULL;
  }

  CloseHandle(winapi_window->render_semaphore);
  DeleteCriticalSection(&winapi_window->event_lock);
  DeleteCriticalSection(&winapi_window->render_lock);

  free(winapi_window);
  window->window = NULL;
  return 0;
}

// Packs hints with a reasonable set of default hints.
void get_default_gl_window_hints(struct gl_window_hints *hints) {
  memset(hints, 0, sizeof(*hints));

  hints->width = 640;
  hints->height = 480;

  hints->fullscreen = 0;
  hints->double_buffered = 1;
}

// Generates a pixel format descriptor that matches the hints.
int get_matching_pixel_format(struct winapi_window *winapi_window,
  const struct gl_window_hints *hints) {
  GLuint pixel_format;

  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW |
    PFD_SUPPORT_OPENGL |
    PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA,
    32,                      // Color depth
    0, 0, 0, 0, 0, 0,        // Color bits
    0,                       // Alpha buffer
    0,                       // Shift bit
    0,                       // Accumulation buffer
    0, 0, 0, 0,              // Accumulation bits
    16,                      // Depth buffer
    0,                       // Stencil buffer
    0,                       // Auxiliary buffer
    PFD_MAIN_PLANE,          // Drawing layer
    0,                       // Reserved
    0, 0, 0                  // Layer masks
  };

  if (!(pixel_format = ChoosePixelFormat(winapi_window->h_dc, &pfd))) {
    debug("get_matching_pixel_format: No matching pixel formats found.\n");
    return 1;
  }

  if (!SetPixelFormat(winapi_window->h_dc, pixel_format, &pfd)) {
    debug("get_matching_pixel_format: Unable to set the pixel format.\n");
    return 1;
  }

  return 0;
}

// Promotes the contents of the back buffer to the front buffer.
int gl_swap_buffers(const struct gl_window *window) {
  struct winapi_window *winapi_window;

  winapi_window = (struct winapi_window *) (window->window);
  return SwapBuffers(winapi_window->h_dc) != TRUE;
}

// Jumps to the entry point for the user interface code.
int gl_window_thread(struct gl_window *gl_window, struct bus_controller *bus) {
  struct winapi_window *winapi_window =
    (struct winapi_window *) (gl_window->window);

  return winapi_window_thread(gl_window, winapi_window, bus);
}

// Informs the caller if an exit was requested.
bool winapi_window_exit_requested(struct winapi_window *window) {
  bool exit_requested;

  EnterCriticalSection(&window->event_lock);
  exit_requested = window->exit_requested;
  LeaveCriticalSection(&window->event_lock);

  return exit_requested;
}

// Handles events that get sent to the window thread.
bool winapi_window_poll_events(struct bus_controller *bus,
  struct winapi_window *winapi_window) {
  bool exit_requested;
  MSG msg;

  exit_requested = false;

  EnterCriticalSection(&winapi_window->event_lock);

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT)
      winapi_window->exit_requested = exit_requested = true;

    else if (msg.message == WM_KEYDOWN) {
      keyboard_press_callback(bus, msg.wParam);
    }

    else if (msg.message == WM_KEYUP) {
      keyboard_release_callback(bus, msg.wParam);
    }

    else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  LeaveCriticalSection(&winapi_window->event_lock);
  return exit_requested;
}

// Copies the frame data to the render thread.
void winapi_window_render_frame(struct winapi_window *window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type) {
  size_t copy_size;

  switch (type & 0x3) {
    case 0:
    case 1:
      copy_size = 0;
      break;

    case 2:
      copy_size = 2;
      break;

    case 3:
      copy_size = 4;
      break;
  }

  copy_size *= xres * yres;

  // Grab the locks and dump the data.
  // TODO: Check to make sure !frame_pending.
  EnterCriticalSection(&window->render_lock);

  memcpy(window->frame_data, data, copy_size);
  window->frame_xres = xres;
  window->frame_yres = yres;
  window->frame_xskip = xskip;
  window->frame_type = type;
  window->frame_pending = true;

  LeaveCriticalSection(&window->render_lock);
}

// Main window thread. Handles and pumps events.
int winapi_window_thread(struct gl_window *gl_window,
  struct winapi_window *winapi_window, struct bus_controller *bus) {
  cen64_time last_report_time;
  cen64_time refresh_timeout;
  unsigned frame_count = 0;

  EnterCriticalSection(&winapi_window->render_lock);

  // Activate the rendering context from THIS thread.
  // Any kind of GL call has to be done from here, or else.
  if (!wglMakeCurrent(winapi_window->h_dc, winapi_window->h_glrc)) {
    debug("winapi_window_thread: Could not attach rendering context.\n");

    return 1;
  }

  gl_window_init(gl_window);
  LeaveCriticalSection(&winapi_window->render_lock);

  get_time(&last_report_time);

  // Poll for input periodically (~1000x a second) while checking
  // to see if the simulator pushed out any frames in that time.
  while (1) {
    DWORD status;

    if (unlikely(frame_count == 60)) {
      winapi_window_update_window_title(winapi_window, &last_report_time);
      frame_count = 0;
    }

    if (winapi_window_poll_events(bus, winapi_window))
      break;

    // Check if we were signaled. If not, just sit around for now.
    EnterCriticalSection(&winapi_window->render_lock);

    if (!winapi_window->frame_pending) {
      status = SignalObjectAndWait(&winapi_window->render_lock,
        winapi_window->render_semaphore, 1, FALSE);


      if (status == WAIT_OBJECT_0)
        EnterCriticalSection(&winapi_window->render_lock);
    }

    else
      status = WAIT_OBJECT_0;

    if (status == WAIT_OBJECT_0) {
//    if (winapi_window->frame_pending && SignalObjectAndWait(
//      &winapi_window->render_lock, winapi_window->render_semaphore,
//      1, FALSE) != WAIT_TIMEOUT) {
      winapi_window->frame_pending = false;
      frame_count++;

      gl_window_render_frame(gl_window, winapi_window->frame_data,
        winapi_window->frame_xres, winapi_window->frame_yres,
        winapi_window->frame_xskip, winapi_window->frame_type);
    }

    LeaveCriticalSection(&winapi_window->render_lock);
  }

  return 0;
}

// Updates the window title.
void winapi_window_update_window_title(
  struct winapi_window *winapi_window, cen64_time *last_report_time) {
  cen64_time current_time;
  unsigned long long ns;
  char window_title[64];

  get_time(&current_time);
  ns = compute_time_difference(&current_time, last_report_time);

  sprintf(window_title,
    "CEN64 ["CEN64_COMPILER" - "CEN64_ARCH_DIR"/"CEN64_ARCH_SUPPORT"]"
    " - %u VI/s", (unsigned) (60 / ((double) ns / NS_PER_SEC)));

  SetWindowText(winapi_window->h_wnd, window_title);
  *last_report_time = current_time;
}

