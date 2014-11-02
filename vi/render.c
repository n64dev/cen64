//
// vi/render.c: Rendering functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/gl_window.h"
#include "os/main.h"

// Initializes OpenGL to an default state.
void gl_window_init(struct gl_window *window) {
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glEnable(GL_TEXTURE_2D);

  // Initialize the texture that we'll use for drawing the screen.
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Initialize vertex arrays for drawing.
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, 0, window->viuv);
  glVertexPointer(2, GL_FLOAT, 0, window->quad);

  window->quad[0] = window->quad[5] =
  window->quad[6] = window->quad[7] = -1;
  window->quad[1] = window->quad[2] =
  window->quad[3] = window->quad[4] = 1;
  window->viuv[2] = window->viuv[4] =
  window->viuv[5] = window->viuv[7] = 1;

  // Tell OpenGL that the byte order is swapped.
  glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
}

// Renders a frame.
void gl_window_render_frame(struct gl_window *gl_window, const uint8_t *buffer,
  unsigned hres, unsigned vres, unsigned hskip, unsigned type) {
  float aspect;

  switch(type) {
    case 0:
      break;

    case 1:
      assert(0 && "Attempted to use reserved frame type.");
      break;

    case 2:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hres + hskip, vres,
        0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, buffer);
      break;

    case 3:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hres + hskip, vres,
        0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
      break;
  }

  aspect = (float) hres / (hres + hskip);
  gl_window->viuv[2] = gl_window->viuv[4] = aspect;

  glDrawArrays(GL_QUADS, 0, 4);
  gl_swap_buffers(gl_window);
}

// Called when the window was resized.
void gl_window_resize_cb(int width, int height) {
  float aspect = 4.0 / 3.0;

  if (height <= 0)
    height = 1;

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if((float) width / (float) height > aspect) {
    aspect = 3.0 / 4.0;
    aspect *= (float)width / (float)height;
    glOrtho(-aspect, aspect, -1, 1, -1, 1);
  }

  else {
    aspect *= (float)height / (float)width;
    glOrtho(-1, 1, -aspect, aspect, -1, 1);
  }

  glClear(GL_COLOR_BUFFER_BIT);
}

