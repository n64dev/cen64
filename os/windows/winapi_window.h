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

struct winapi_window {
  HINSTANCE h_instance;
  HWND h_wnd;

  HDC h_dc;
  HGLRC h_glrc;

  bool exit_requested;
};

void winapi_window_poll_events(struct bus_controller *bus);

#endif

