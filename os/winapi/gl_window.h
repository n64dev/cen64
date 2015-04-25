//
// os/winapi/gl_window.h: WinAPI/OpenGL window definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_WINAPI_GL_WINDOW
#define CEN64_OS_WINAPI_GL_WINDOW
#include "gl_common.h"
#include "gl_config.h"
#include "gl_display.h"
#include "gl_screen.h"
#include <stddef.h>
#include <stdlib.h>
#include <windows.h>

#define CEN64_GL_WINDOW_BAD (NULL)
struct cen64_gl_window {
  HINSTANCE hinstance;
  HWND hwnd;
  HDC hdc;

  int pixel_format;
};

typedef struct cen64_gl_window *cen64_gl_window;

LRESULT CALLBACK WndProc(HWND hwnd,
  UINT message, WPARAM w_param, LPARAM l_param);

//
// Creates a (hidden) cen64_gl_window.
//
static inline cen64_gl_window cen64_gl_window_create(
  cen64_gl_display display, cen64_gl_screen screen,
  const cen64_gl_config *config, const char *title) {
  cen64_gl_window window;

  WNDCLASS wc; 
  DWORD dw_ex_style;
  DWORD dw_style;
  RECT window_rect;

  if ((window = malloc(sizeof(*window))) == NULL)
    return CEN64_GL_WINDOW_BAD;

  window_rect.left = 0;
  window_rect.right = 640;
  window_rect.top = 0;
  window_rect.bottom = 480;

  window->hinstance = GetModuleHandle(NULL);
  window->pixel_format = config->pixel_format;

  memset(&wc, 0, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = (WNDPROC) WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = window->hinstance;
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "CEN64";

  if (!RegisterClass(&wc)) {
    free(window);

    return CEN64_GL_WINDOW_BAD;
  }

  dw_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
  dw_style = WS_OVERLAPPEDWINDOW;
  AdjustWindowRectEx(&window_rect, dw_style, FALSE, dw_ex_style);

  if ((window->hwnd = CreateWindowEx(dw_ex_style, "CEN64", title,
    dw_style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0,
    window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
    NULL, NULL, window->hinstance, NULL)) == NULL) {
    UnregisterClass("CEN64", window->hinstance);
    free(window);

    return CEN64_GL_WINDOW_BAD;
  }

  // Store the pointer to the gl_window in the HWND attribute.
  // We'll need it later to perform callbacks on resize and such.
  // Also grab the window's device context so we can create a GL RC.
  SetLastError(0);
  SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (uintptr_t) window);

  if (GetLastError() || (window->hdc = GetDC(window->hwnd)) == NULL) {
    DestroyWindow(window->hwnd);
    UnregisterClass("CEN64", window->hinstance);
    free(window);

    return CEN64_GL_WINDOW_BAD;
  }

  SetPixelFormat(window->hdc, window->pixel_format, &config->pfd);
  return window;
}

//
// Releases resources allocated by cen64_gl_window_create.
//
static inline void cen64_gl_window_destroy(cen64_gl_window window) {
  ReleaseDC(window->hwnd, window->hdc);
  DestroyWindow(window->hwnd);
  UnregisterClass("CEN64", window->hinstance);

  free(window);
}

//
// Swaps the front and back buffers of the cen65_gl_window.
//
static inline void cen64_gl_window_swap_buffers(cen64_gl_window window) {
  SwapBuffers(window->hdc);
}

//
// Unhides the cen64_gl_window.
//
static inline void cen64_gl_window_unhide(cen64_gl_window window) {
  ShowWindow(window->hwnd, SW_SHOW);
}

#endif

