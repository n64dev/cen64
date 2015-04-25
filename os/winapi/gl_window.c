//
// os/winapi/gl_window.h: WinAPI/OpenGL window definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "gl_common.h"
#include "gl_window.h"
#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd,
  UINT message, WPARAM w_param, LPARAM l_param) {
  cen64_gl_window window;

  switch(message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    case WM_SIZE:
      window = (cen64_gl_window) GetWindowLongPtr(hwnd, GWLP_USERDATA);
      break;

    default:
      return DefWindowProc(hwnd, message, w_param, l_param);
  }

  return 0;
}

