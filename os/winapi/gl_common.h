//
// os/winapi/gl_common.h: Common WinAPI/OpenGL header.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_WINAPI_GL_COMMON
#define CEN64_OS_WINAPI_GL_COMMON
#include "common.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

enum cen64_gl_context_type {
  CEN64_GL_CONTEXT_TYPE_RGBA           = PFD_TYPE_RGBA,
  CEN64_GL_CONTEXT_TYPE_COLOR_INDEX    = PFD_TYPE_COLORINDEX
};

enum cen64_gl_drawable_type {
  CEN64_GL_DRAWABLE_TYPE_WINDOW        = PFD_DRAW_TO_WINDOW,
  CEN64_GL_DRAWABLE_TYPE_BITMAP        = PFD_DRAW_TO_BITMAP
};

enum cen64_gl_layer_type {
  CEN64_GL_LAYER_TYPE_DEFAULT          = PFD_MAIN_PLANE,
  CEN64_GL_LAYER_TYPE_OVERLAY          = PFD_OVERLAY_PLANE,
  CEN64_GL_LAYER_TYPE_UNDERLAY         = PFD_UNDERLAY_PLANE
};

#endif

