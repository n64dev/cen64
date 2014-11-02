//
// vi/controller.c: Video interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "device.h"
#include "os/gl_window.h"
#include "os/main.h"
#include "os/timer.h"
#include "ri/controller.h"
#include "vi/controller.h"
#include "vr4300/interface.h"

#define VI_COUNTER_START (62500000.0 / 60.0) + 1;

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *vi_register_mnemonics[NUM_VI_REGISTERS] = {
#define X(reg) #reg,
#include "vi/registers.md"
#undef X
};
#endif

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

// Reads a word from the VI MMIO register space.
int read_vi_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct vi_controller *vi = (struct vi_controller *) opaque;
  unsigned offset = address - VI_REGS_BASE_ADDRESS;
  enum vi_register reg = (offset >> 2);

  // TODO: Possibly a giant hack.
  if (vi->regs[VI_V_SYNC_REG] > 0) {
    vi->regs[VI_CURRENT_REG] =
      (((62500000.0f / 60.0f) + 1) - (vi->counter)) /
      (((62500000.0f / 60.0f) + 1) / vi->regs[VI_V_SYNC_REG]);

    vi->regs[VI_CURRENT_REG] &= ~0x1;
  }

  else
    vi->regs[VI_CURRENT_REG] = 0;

  *word = vi->regs[reg];
  debug_mmio_read(vi, vi_register_mnemonics[reg], *word);
  return 0;
}

// Advances the controller by one clock cycle.
void vi_cycle(struct vi_controller *vi) {
  struct render_area *ra = &vi->render_area;
  int hskip, vres, hres;
  float hcoeff, vcoeff;

  const uint8_t *buffer;
  uint32_t offset;

  if (likely(vi->counter-- != 0))
    return;

  offset = vi->regs[VI_ORIGIN_REG] & 0xFFFFFF;
  buffer = vi->bus->ri->ram + offset;

  // We're at a vertical interrupt...
  // check if an exit was requested.
  if (os_exit_requested(&vi->gl_window))
    device_request_exit(vi->bus);

  // Calculate the bounding positions.
  ra->x.start = vi->regs[VI_H_START_REG] >> 16 & 0x3FF;
  ra->x.end = vi->regs[VI_H_START_REG] & 0x3FF;
  ra->y.start = vi->regs[VI_V_START_REG] >> 16 & 0x3FF;
  ra->y.end = vi->regs[VI_V_START_REG] & 0x3FF;

  hcoeff = (float) (vi->regs[VI_X_SCALE_REG] & 0xFFF) / (1 << 10);
  vcoeff = (float) (vi->regs[VI_Y_SCALE_REG] & 0xFFF) / (1 << 10);

  // Calculate the height and width of the frame.
  vres = ra->height =((ra->y.end - ra->y.start) >> 1) * vcoeff;
  hres = ra->width = ((ra->x.end - ra->x.start)) * hcoeff;
  hskip = ra->hskip = vi->regs[VI_WIDTH_REG] - ra->width;

  if (hres > 0 && vres > 0) {
    if (hres > 640) {
      hskip += (hres - 640);
      hres = 640;
    }

    if (vres > 480)
       vres = 480;

    os_render_frame(&vi->gl_window, buffer, hres, vres,
      hskip, vi->regs[VI_STATUS_REG] & 0x3);
  }

  // Raise an interrupt to indicate refresh.
  signal_rcp_interrupt(vi->bus->vr4300, MI_INTR_VI);
    vi->counter = VI_COUNTER_START;
}

// Initializes the VI.
int vi_init(struct vi_controller *vi,
  struct bus_controller *bus) {
  vi->counter = VI_COUNTER_START;
  vi->bus = bus;

  return 0;
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

// Writes a word to the VI MMIO register space.
int write_vi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct vi_controller *vi = (struct vi_controller *) opaque;
  unsigned offset = address - VI_REGS_BASE_ADDRESS;
  enum vi_register reg = (offset >> 2);

  debug_mmio_write(vi, vi_register_mnemonics[reg], word, dqm);

  if (reg == VI_CURRENT_REG)
    clear_rcp_interrupt(vi->bus->vr4300, MI_INTR_VI);

  else {
    vi->regs[reg] &= ~dqm;
    vi->regs[reg] |= word;
  }

  return 0;
}

