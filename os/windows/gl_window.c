//
// os/windows/gl_window.c
//
// Convenience functions for managing rendering windows.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common/debug.h"
#include "os/gl_window.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <tchar.h>
#include <GL/gl.h>

extern DWORD event_thread_id;
static const wchar_t CLASS_NAME[] = L"lekki";

struct winapi_window {
  HINSTANCE h_instance;
  HWND h_wnd;

  HDC h_dc;
  HGLRC h_glrc;
};

static int create_gl_context(struct winapi_window *winapi_window, HGLRC *h_rc);

static int get_matching_pixel_format(struct winapi_window *winapi_window,
  const struct gl_window_hints *hints);

/* Callback function to handle message sent to the window. */
LRESULT CALLBACK WndProc(HWND hWnd,
  UINT message, WPARAM wParam, LPARAM lParam) {
  switch(message) {
    case WM_DESTROY:
      PostThreadMessage(event_thread_id, WM_QUIT, 0, 0);
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

/* Creates a new rendering context. */
int create_gl_context(struct winapi_window *winapi_window, HGLRC *h_rc) {
  if ((*h_rc = wglCreateContext(winapi_window->h_dc)) == NULL) {
    debug("create_gl_context: wglCreateContext returned NULL.");
    return 1;
  }

  return 0;
}

/* Creates a new rendering window. */
int create_gl_window(const char *window_title, struct gl_window *gl_window,
  const struct gl_window_hints *hints) {
  struct winapi_window *winapi_window;
  int fullscreen;

  WNDCLASS wc;
  DWORD dw_ex_style;
  DWORD dw_style;
  RECT window_rect;

  fullscreen = hints->fullscreen;

  window_rect.left = 0;
  window_rect.right = hints->width;
  window_rect.top = 0;
  window_rect.bottom = hints->height;

  /* Allocate memory for the opaque handle inside thw window. */
  if ((gl_window->window = malloc(sizeof(*winapi_window))) == NULL) {
    debug("create_gl_window: Could not allocate enough memory.\n");
    return 1;
  }

  winapi_window = (struct winapi_window *) (gl_window->window);
  memset(winapi_window, 0, sizeof(*winapi_window));

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
  wc.lpszClassName = CLASS_NAME;

  if (!RegisterClass(&wc)) {
    debug("create_gl_window: Failed to register the window class.\n");
    goto create_out_destroy;
  }

  dw_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
  dw_style = WS_OVERLAPPEDWINDOW;
  AdjustWindowRectEx(&window_rect, dw_style, FALSE, dw_ex_style);

  if (!(winapi_window->h_wnd = CreateWindowEx(dw_ex_style,
    CLASS_NAME, CLASS_NAME, dw_style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0,
    window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
    NULL, NULL, winapi_window->h_instance, NULL))) {
    debug("create_gl_window: Failed to create the window.\n");
    goto create_out_destroy;
  }

  if ((winapi_window->h_dc = GetDC(winapi_window->h_wnd)) == NULL) {
    debug("create_gl_window: Failed get an OpenGL device context.\n");
    goto create_out_destroy;
  }

  if (get_matching_pixel_format(winapi_window, hints)) {
    debug("create_gl_window: Failed to match window hints.\n");
    goto create_out_destroy;
  }

  if (create_gl_context(winapi_window, &winapi_window->h_glrc)) {
    debug("create_gl_window: Failed to acquire a GL context.\n");
    goto create_out_destroy;
  }

  if (!wglMakeCurrent(winapi_window->h_dc, winapi_window->h_glrc)) {
    debug("create_glx_window: Could not attach rendering context.\n");
    goto create_out_destroy;
  }

  ShowWindow(winapi_window->h_wnd, SW_SHOW);
  SetForegroundWindow(winapi_window->h_wnd);
  SetFocus(winapi_window->h_wnd);
  return 0;

create_out_destroy:
  destroy_gl_window(gl_window);
  return 1;
}

/* Destroys an existing rendering window. */
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
    UnregisterClass(CLASS_NAME, winapi_window->h_instance);
    winapi_window->h_instance = NULL;
  }

  free(winapi_window);
  window->window = NULL;
  return 0;
}

/* Packs hints with a reasonable set of default hints. */
void get_default_gl_window_hints(struct gl_window_hints *hints) {
  memset(hints, 0, sizeof(*hints));

  hints->width = 800;
  hints->height = 600;
  hints->double_buffered = 1;
}

/* Generates a pixel format descriptor that matches the hints. */
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

/* Promotes the contents of the back buffer to the front buffer. */
void gl_swap_buffers(const struct gl_window *window) {
  struct winapi_window *winapi_window;

  winapi_window = (struct winapi_window *) (window->window);
  SwapBuffers(winapi_window->h_dc);
}

