//
// os/winapi/gl_window.h: WinAPI/OpenGL window definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device/device.h"
#include "gl_common.h"
#include "gl_window.h"
#include "thread.h"
#include "timer.h"
#include "os/common/input.h"
#include "vi/controller.h"
#include "vi/render.h"
#include <windows.h>

static bool cen64_gl_window_pump_events(struct vi_controller *vi,
  cen64_time *time, unsigned *frame_count);

static LRESULT CALLBACK WndProc(HWND hwnd,
  UINT message, WPARAM w_param, LPARAM l_param) {

  switch(message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    case WM_SIZE:
      gl_window_resize_cb(LOWORD(l_param), HIWORD(l_param));
      break;

    default:
      return DefWindowProc(hwnd, message, w_param, l_param);
  }

  return 0;
}

// Handles events that come from Windows.
bool cen64_gl_window_pump_events(struct vi_controller *vi,
  cen64_time *last_update_time, unsigned *frame_count) {
  struct bus_controller *bus;
  bool exit_requested = false;
  MSG msg;

  memcpy(&bus, vi, sizeof(bus));

  while (1) {
    GetMessage(&msg, NULL, 0, 0);

    if (msg.message == WM_QUIT) {
      cen64_mutex_lock(&vi->window->event_mutex);
      vi->window->exit_requested = exit_requested = true;
      cen64_mutex_unlock(&vi->window->event_mutex);
      break;
    }

    else if (msg.message == WM_KEYDOWN) {
      cen64_mutex_lock(&vi->window->event_mutex);
      keyboard_press_callback(bus, msg.wParam);
      cen64_mutex_unlock(&vi->window->event_mutex);
    }

    else if (msg.message == WM_KEYUP) {
      cen64_mutex_lock(&vi->window->event_mutex);
      keyboard_release_callback(bus, msg.wParam);
      cen64_mutex_unlock(&vi->window->event_mutex);
    }

    else if (msg.message == WM_USER) {
      cen64_gl_window window = vi->window;

      cen64_mutex_lock(&window->render_mutex);

      gl_window_render_frame(vi, window->frame_buffer,
        window->frame_hres, window->frame_vres,
        window->frame_hskip, window->frame_type);

      cen64_mutex_unlock(&window->render_mutex);

      // Update the window title every 60 VIs
      // to display the current VI/s rate.
      if (++(*frame_count) == 60) {
        char title[128];
        cen64_time current_time;
        float ns;

        // Compute time spent rendering last 60 frames, reset timer/counter.
        get_time(&current_time);
        ns = compute_time_difference(&current_time, last_update_time);
        *last_update_time = current_time;
        *frame_count = 0;

        sprintf(title,
          "CEN64 ["CEN64_COMPILER" - "CEN64_ARCH_DIR"/"CEN64_ARCH_SUPPORT"]"
          " - %.1f VI/s", (60 / (ns / NS_PER_SEC)));

        cen64_gl_window_set_title(window, title);
      }
    }

    else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return exit_requested;
}

cen64_gl_window cen64_gl_window_create(
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
  window_rect.bottom = 474;

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

  window->exit_requested = false;
  cen64_mutex_create(&window->event_mutex);
  cen64_mutex_create(&window->render_mutex);
  window->thread_id = GetCurrentThreadId();
  return window;
}

// Thread that controls the user interface, etc.
int cen64_gl_window_thread(struct cen64_device *device) {
  cen64_time last_update_time;
  unsigned frame_count;

  get_time(&last_update_time);
  frame_count = 0;

  while (!cen64_gl_window_pump_events(&device->vi,
    &last_update_time, &frame_count));

  return 0;
}

