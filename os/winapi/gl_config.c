//
// os/winapi/gl_config.c: WinAPI/OpenGL framebuffer configuration.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "gl_common.h"
#include "gl_config.h"
#include "gl_hints.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

//
// Creates a matching cen64_gl_config from a cen64_gl_hints struct.
//
// On error, CEN64_GL_CONFIG_BAD is returned.
//
cen64_gl_config *cen64_gl_config_create(cen64_gl_display display,
  cen64_gl_screen screen, const cen64_gl_hints *hints, int *matching) {
  struct cen64_gl_config *config;
  PIXELFORMATDESCRIPTOR pfd;

  // Do *NOT* use this hDC HANDLE outside this function.
  // It is the desktop window's hDC, and we shouldn't mess
  // with it. The only thing it's used for is to match a
  // PIXELFORMATDESCRIPTOR and expose success/failure.
  HDC screen_hdc;

  if ((config = malloc(sizeof(*config))) == NULL)
    return CEN64_GL_CONFIG_BAD;

  if ((screen_hdc = GetDC(screen)) == NULL) {
    free(config);

    return CEN64_GL_CONFIG_BAD;
  }

  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(pfd);

  // As of 04/25/15, the MSDN documention (still) just says
  // to set this field to 1. So, uh, let's go with that.
  pfd.nVersion = 1;

  // Pack the structure using the provided hints->
  pfd.dwFlags = PFD_SUPPORT_OPENGL;
  pfd.iLayerType = PFD_MAIN_PLANE;

  pfd.iPixelType |= (enum cen64_gl_context_type) hints->context_type;
  pfd.dwFlags |= (enum cen64_gl_drawable_type) hints->drawable_type;

  if (hints->double_buffered != -1)
    pfd.dwFlags |= PFD_DOUBLEBUFFER;

  if (hints->stereoscopic != -1)
    pfd.dwFlags |= PFD_STEREO;

  if (hints->rgb_color_depth != -1)
    pfd.cColorBits = hints->rgb_color_depth;

  if (hints->alpha_color_depth != -1)
    pfd.cAlphaBits = hints->alpha_color_depth;

  if (hints->depth_buffer_size != -1)
    pfd.cDepthBits = hints->depth_buffer_size;

  if (hints->num_aux_buffers != -1)
    pfd.cAuxBuffers = hints->num_aux_buffers;

  if (hints->stencil_buffer_size != -1)
    pfd.cStencilBits = hints->stencil_buffer_size;

  if (hints->accum_buffer_red_bits != -1)
    pfd.cAccumRedBits = hints->accum_buffer_red_bits;

  if (hints->accum_buffer_green_bits != -1)
    pfd.cAccumGreenBits = hints->accum_buffer_green_bits;

  if (hints->accum_buffer_blue_bits != -1)
    pfd.cAccumBlueBits = hints->accum_buffer_blue_bits;

  if (hints->accum_buffer_alpha_bits != -1)
    pfd.cAccumAlphaBits = hints->accum_buffer_alpha_bits;

  config->pixel_format = ChoosePixelFormat(screen_hdc, &pfd);
  DescribePixelFormat(screen_hdc, config->pixel_format,
    sizeof(config->pfd), &config->pfd);

  ReleaseDC(screen, screen_hdc);

  *matching = 1;
  return config;
}

