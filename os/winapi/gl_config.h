//
// os/winapi/gl_config.h: WinAPI/OpenGL framebuffer configuration.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_WINAPI_GL_CONFIG
#define CEN64_OS_WINAPI_GL_CONFIG
#include "gl_common.h"
#include "gl_display.h"
#include "gl_hints.h"
#include "gl_screen.h"
#include <stddef.h>
#include <stdlib.h>
#include <windows.h>

#define CEN64_GL_CONFIG_BAD (NULL)
struct cen64_gl_config {
  PIXELFORMATDESCRIPTOR pfd;
  int pixel_format;
};

typedef struct cen64_gl_config cen64_gl_config;

//
// Creates a matching cen64_gl_config from a cen64_gl_hints struct.
//
// On error, CEN64_GL_CONFIG_BAD is returned. On success, something
// other than CEN64_GL_CONFIG_BAD is returned, and matching is set
// to indicate the number of matches present in the returned array.
//
cen64_gl_config *cen64_gl_config_create(cen64_gl_display display,
  cen64_gl_screen screen, const cen64_gl_hints *hints, int *matching);

//
// Releases resources allocated by cen64_gl_config_create.
//
static inline void cen64_gl_config_destroy(cen64_gl_config *config) {
  free(config);
}

//
// Wrappers for querying for features/types.
//
static inline enum cen64_gl_context_type cen64_gl_config_get_context_type(
  cen64_gl_display display, cen64_gl_config *config) {
  return (enum cen64_gl_context_type) config->pfd.iPixelType;
}

static inline enum cen64_gl_drawable_type cen64_gl_config_get_drawable_type(
  cen64_gl_display display, cen64_gl_config *config) {
  return (config->pfd.dwFlags & PFD_DRAW_TO_BITMAP)
    ? CEN64_GL_DRAWABLE_TYPE_BITMAP
    : CEN64_GL_DRAWABLE_TYPE_WINDOW;
}

static inline enum cen64_gl_layer_type cen64_gl_config_get_layer_type(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.iLayerType;
}

static inline int cen64_gl_config_is_double_buffered(
  cen64_gl_display display, cen64_gl_config *config) {
  return (config->pfd.dwFlags & PFD_DOUBLEBUFFER) != 0;
}

static inline int cen64_gl_config_is_renderable(
  cen64_gl_display display, cen64_gl_config *config) {
  return (config->pfd.dwFlags & PFD_SUPPORT_OPENGL) != 0;
}

static inline int cen64_gl_config_is_stereoscopic(
  cen64_gl_display display, cen64_gl_config *config) {
  return (config->pfd.dwFlags & PFD_STEREO) != 0;
}

//
// Wrappers for querying for color depths.
//
static inline int cen64_gl_config_get_color_depth(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cColorBits;
}

static inline int cen64_gl_config_get_red_color_depth(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cRedBits;
}

static inline int cen64_gl_config_get_green_color_depth(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cGreenBits;
}

static inline int cen64_gl_config_get_blue_color_depth(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cBlueBits;
}

static inline int cen64_gl_config_get_alpha_color_depth(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cAlphaBits;
}

//
// Wrappers for querying for buffer sizes, counts.
//
static inline int cen64_gl_config_get_depth_buffer_count(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cDepthBits;
}

static inline int cen64_gl_config_get_num_auxiliary_buffers(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cAuxBuffers;
}

static inline int cen64_gl_config_get_stencil_buffer_size(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cStencilBits;
}

//
// Wrappers for querying for accumulation buffer bits.
//
static inline int cen64_gl_config_get_red_accum_buffer_bits(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cAccumRedBits;
}

static inline int cen64_gl_config_get_blue_accum_buffer_bits(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cAccumBlueBits;
}

static inline int cen64_gl_config_get_green_accum_buffer_bits(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cAccumGreenBits;
}

static inline int cen64_gl_config_get_alpha_accum_buffer_bits(
  cen64_gl_display display, cen64_gl_config *config) {
  return config->pfd.cAccumAlphaBits;
}

#endif

