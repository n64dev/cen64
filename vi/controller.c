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
#include "device/device.h"
#include "os/main.h"
#include "ri/controller.h"
#include "vi/controller.h"
#include "vr4300/interface.h"

#include "gl_common.h"
#include "gl_context.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "gl_window.h"

#define VI_COUNTER_START (62500000.0 / 60.0) + 1;

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *vi_register_mnemonics[NUM_VI_REGISTERS] = {
#define X(reg) #reg,
#include "vi/registers.md"
#undef X
};
#endif

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
  unsigned type;

  const uint8_t *buffer;
  uint32_t offset;

  if (likely(vi->counter-- != 0))
    return;

  offset = vi->regs[VI_ORIGIN_REG] & 0xFFFFFF;
  buffer = vi->bus->ri->ram + offset;

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
  type = vi->regs[VI_STATUS_REG] & 0x3;

  if (hres <= 0 || vres <= 0)
    type = 0;

  // Interact with the user interface?
  cen64_gl_window_pump_events(vi);
  gl_window_render_frame(vi, buffer, hres, vres, hskip, type);
#if 0
  if (likely(vi->gl_window.window)) {
    if (os_exit_requested(&vi->gl_window))
      device_exit(vi->bus);

    os_render_frame(&vi->gl_window, buffer, hres, vres, hskip, type);
  }

  else if (device_exit_requested)
    device_exit(vi->bus);
#endif

  // Raise an interrupt to indicate refresh.
  signal_rcp_interrupt(vi->bus->vr4300, MI_INTR_VI);
  vi->counter = VI_COUNTER_START;
}

// Initializes the VI.
int vi_init(struct vi_controller *vi, struct bus_controller *bus) {
  vi->counter = VI_COUNTER_START;
  vi->bus = bus;

  // Create a window for rendering. If we're successful,
  // we'll work our way into the nested statements and
  // return success.
  if ((vi->display = cen64_gl_display_create(
    NULL)) != CEN64_GL_DISPLAY_BAD) {

    if ((vi->screen = cen64_gl_screen_create(
      vi->display, -1)) != CEN64_GL_SCREEN_BAD) {
      struct cen64_gl_hints hints = cen64_default_gl_hints;
      struct cen64_gl_config *config;
      int num_matching;

      if ((config = cen64_gl_config_create(vi->display, vi->screen,
        &hints, &num_matching)) != CEN64_GL_CONFIG_BAD) {

        if ((vi->window = cen64_gl_window_create(vi->display, vi->screen,
          config, "CEN64")) != CEN64_GL_WINDOW_BAD) {
          cen64_gl_config_destroy(config);

          if ((vi->context = cen64_gl_context_create(
            vi->window)) != CEN64_GL_CONTEXT_BAD) {
            cen64_gl_window_unhide(vi->window);

            gl_window_init(vi);
            return 0;
          }
        }

  // Something failed, and it's hard to perform RAII in C.
  // So now we release resources and return an error.
        cen64_gl_window_destroy(vi->window);
        cen64_gl_config_destroy(config);
      }

      cen64_gl_screen_destroy(vi->screen);
    }

    cen64_gl_display_destroy(vi->display);
  }

  return -1;
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

