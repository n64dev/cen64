//
// os/x11/gl_config.c: X11/OpenGL framebuffer configuration.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "gl_common.h"
#include "gl_config.h"
#include "gl_hints.h"

//
// Creates a matching cen64_gl_config from a cen64_gl_hints struct.
//
// On error, CEN64_GL_CONFIG_BAD is returned.
//
cen64_gl_config *cen64_gl_config_create(cen64_gl_display display,
  cen64_gl_screen screen, const cen64_gl_hints *hints, int *matching) {
  int idx = 0;

  // Magic number was chosen based on the glXChooseFBConfig man page.
  // It is at least large enough to hold the supported attributes, as
  // well as a few additional ones. Expand it at your convenience.
  int attribute_list[64];

  // Build the attributes list using the provided hints.
  attribute_list[idx++] = GLX_X_RENDERABLE;
  attribute_list[idx++] = True;

  attribute_list[idx++] = GLX_X_VISUAL_TYPE;
  attribute_list[idx++] = GLX_TRUE_COLOR;

  attribute_list[idx++] = GLX_RENDER_TYPE;
  attribute_list[idx++] = hints->context_type;

  attribute_list[idx++] = GLX_DRAWABLE_TYPE;
  attribute_list[idx++] = hints->drawable_type;

  if (hints->double_buffered != -1) {
    attribute_list[idx++] = GLX_DOUBLEBUFFER;
    attribute_list[idx++] = hints->double_buffered ? True : False;
  }

  if (hints->stereoscopic != -1) {
    attribute_list[idx++] = GLX_STEREO;
    attribute_list[idx++] = hints->stereoscopic ? True : False;
  }

  if (hints->rgb_color_depth != -1) {
    int component_depth = hints->rgb_color_depth > 0
      ? hints->rgb_color_depth / 3
      : 0;

    attribute_list[idx++] = GLX_RED_SIZE;
    attribute_list[idx++] = component_depth;

    attribute_list[idx++] = GLX_GREEN_SIZE;
    attribute_list[idx++] = component_depth;

    attribute_list[idx++] = GLX_BLUE_SIZE;
    attribute_list[idx++] = component_depth;
  }

  if (hints->alpha_color_depth != -1) {
    attribute_list[idx++] = GLX_ALPHA_SIZE;
    attribute_list[idx++] = hints->alpha_color_depth;
  }

  if (hints->depth_buffer_size != -1) {
    attribute_list[idx++] = GLX_DEPTH_SIZE;
    attribute_list[idx++] = hints->depth_buffer_size;
  }

  if (hints->num_aux_buffers != -1) {
    attribute_list[idx++] = GLX_AUX_BUFFERS;
    attribute_list[idx++] = hints->num_aux_buffers;
  }

  if (hints->stencil_buffer_size != -1) {
    attribute_list[idx++] = GLX_STENCIL_SIZE;
    attribute_list[idx++] = hints->stencil_buffer_size;
  }

  if (hints->accum_buffer_red_bits != -1) {
    attribute_list[idx++] = GLX_ACCUM_RED_SIZE;
    attribute_list[idx++] = hints->accum_buffer_red_bits;
  }

  if (hints->accum_buffer_green_bits != -1) {
    attribute_list[idx++] = GLX_ACCUM_GREEN_SIZE;
    attribute_list[idx++] = hints->accum_buffer_green_bits;
  }

  if (hints->accum_buffer_blue_bits != -1) {
    attribute_list[idx++] = GLX_ACCUM_BLUE_SIZE;
    attribute_list[idx++] = hints->accum_buffer_blue_bits;
  }

  if (hints->accum_buffer_alpha_bits != -1) {
    attribute_list[idx++] = GLX_ACCUM_ALPHA_SIZE;
    attribute_list[idx++] = hints->accum_buffer_alpha_bits;
  }

  // Terminate the list, as required by X11.
  attribute_list[idx++] = None;

  return glXChooseFBConfig(display, screen, attribute_list, matching);
}

//
// Fetches an attribute from the cen64_gl_config object.
//
int cen64_gl_config_fetch_attribute(cen64_gl_display display,
  cen64_gl_config *config, int what) {
  int value, status;

  status = glXGetFBConfigAttrib(display, *config, what, &value);
  if (status == GLX_NO_EXTENSION || status == GLX_BAD_ATTRIBUTE)
    return -1;

  return value;
}

