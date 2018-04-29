//
// vi/render.c: Rendering functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/gl_window.h"
#include "os/main.h"

// Initializes OpenGL to an default state.
void gl_window_init(struct vi_controller *vi) {
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glEnable(GL_TEXTURE_2D);

  // Initialize gamma boost blend function.
  glBlendFunc(GL_ONE, GL_ONE);

  // Initialize the texture that we'll use for drawing the screen.
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // Initialize vertex arrays for drawing.
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, 0, vi->viuv);
  glVertexPointer(2, GL_FLOAT, 0, vi->quad);

  vi->quad[0] = vi->quad[5] =
  vi->quad[6] = vi->quad[7] = -1;
  vi->quad[1] = vi->quad[2] =
  vi->quad[3] = vi->quad[4] = 1;
  vi->viuv[2] = vi->viuv[4] =
  vi->viuv[5] = vi->viuv[7] = 1;

  // Tell OpenGL that the byte order is swapped.
  glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
}

// Renders a frame.
void gl_window_render_frame(struct vi_controller *vi, const uint8_t *buffer,
  unsigned hres, unsigned vres, unsigned hskip, unsigned type) {
  float aspect;

  switch(type) {
    case 0:
      return;

    case 1:
      assert(0 && "Attempted to use reserved frame type.");
      return;

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
  vi->viuv[2] = vi->viuv[4] = aspect;

  glDrawArrays(GL_QUADS, 0, 4);

  // Gamma boost blend.
  if (vi->regs[VI_STATUS_REG] & 0x8) {
    glEnable(GL_BLEND);
    glColor3f(0.15f, 0.15f, 0.15f);
    glDrawArrays(GL_QUADS, 0, 4); // Texture quad blend

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.1f, 0.1f, 0.1f);
    glDrawArrays(GL_QUADS, 0, 4); // White quad blend

    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
  }

  cen64_gl_window_swap_buffers(vi->window);
}

// Called when the window was resized.
void gl_window_resize_cb(int width, int height) {
  float aspect = 640.0 / 474.0;

  if (height <= 0)
    height = 1;

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if((float) width / (float) height > aspect) {
    aspect = 474.0 / 640.0;
    aspect *= (float)width / (float)height;
    glOrtho(-aspect, aspect, -1, 1, -1, 1);
  }

  else {
    aspect *= (float)height / (float)width;
    glOrtho(-1, 1, -aspect, aspect, -1, 1);
  }

  glClear(GL_COLOR_BUFFER_BIT);
}

