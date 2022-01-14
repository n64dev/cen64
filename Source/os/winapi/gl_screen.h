//
// os/winapi/gl_screen.h: WinAPI screen definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_WINAPI_GL_SCREEN
#define CEN64_OS_WINAPI_GL_SCREEN
#include "gl_common.h"
#include <stddef.h>
#include <windows.h>

#define CEN64_GL_SCREEN_BAD (NULL)
typedef HWND cen64_gl_screen;

//
// Creates a cen64_gl_screen object on a given cen64_gl_display.
//
// 'which' specifies which screen on a display to use (from 0 to
// num_screens - 1). If 'which' is less than zero, it is treated as
// a don't care.
//
// On error, CEN64_GL_SCREEN_BAD is returned.
//
static inline cen64_gl_screen cen64_gl_screen_create(
  cen64_gl_display display, int which) {
  if (which >= 0)
    return NULL; // TODO

  return GetDesktopWindow();
}

//
// Releases resources allocated by cen64_screen_create.
//
static inline void cen64_gl_screen_destroy(cen64_gl_screen screen) {}

#endif

