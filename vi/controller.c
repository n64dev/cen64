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
#include "vi/render.h"
#include "vi/window.h"
#include "vr4300/interface.h"

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
  cen64_gl_window window;
  size_t copy_size;

  struct render_area *ra = &vi->render_area;
  struct bus_controller *bus;
  float hcoeff, vcoeff;

  if (likely(vi->counter-- != 0))
    return;

  window = vi->window;

  // Calculate the bounding positions.
  ra->x.start = vi->regs[VI_H_START_REG] >> 16 & 0x3FF;
  ra->x.end = vi->regs[VI_H_START_REG] & 0x3FF;
  ra->y.start = vi->regs[VI_V_START_REG] >> 16 & 0x3FF;
  ra->y.end = vi->regs[VI_V_START_REG] & 0x3FF;

  hcoeff = (float) (vi->regs[VI_X_SCALE_REG] & 0xFFF) / (1 << 10);
  vcoeff = (float) (vi->regs[VI_Y_SCALE_REG] & 0xFFF) / (1 << 10);

  // Interact with the user interface?
  if (likely(window)) {
    cen64_mutex_lock(&window->event_mutex);

    if (unlikely(window->exit_requested)) {
      cen64_mutex_unlock(&window->event_mutex);
      device_exit(vi->bus);
    }

    cen64_mutex_unlock(&window->event_mutex);
    cen64_mutex_lock(&window->render_mutex);

    // Calculate the height and width of the frame.
    window->frame_vres = ra->height =((ra->y.end - ra->y.start) >> 1) * vcoeff;
    window->frame_hres = ra->width = ((ra->x.end - ra->x.start)) * hcoeff;
    window->frame_hskip = ra->hskip = vi->regs[VI_WIDTH_REG] - ra->width;
    window->frame_type = vi->regs[VI_STATUS_REG] & 0x3;

    if (window->frame_hres <= 0 || window->frame_vres <= 0)
      window->frame_type = 0;

    // Copy the frame data into a temporary buffer.
    copy_size = sizeof(bus->ri->ram) - (vi->regs[VI_ORIGIN_REG] & 0xFFFFFF);

    if (copy_size > sizeof(vi->window->frame_buffer))
      copy_size = sizeof(vi->window->frame_buffer);

    memcpy(&bus, vi, sizeof(bus));
    memcpy(vi->window->frame_buffer,
      bus->ri->ram + (vi->regs[VI_ORIGIN_REG] & 0xFFFFFF),
      copy_size);

    cen64_mutex_unlock(&vi->window->render_mutex);
    cen64_gl_window_push_frame(window);
  }

  // Raise an interrupt to indicate refresh.
  signal_rcp_interrupt(vi->bus->vr4300, MI_INTR_VI);
  vi->counter = VI_COUNTER_START;
}

// Initializes the VI.
int vi_init(struct vi_controller *vi, struct bus_controller *bus) {
  vi->counter = VI_COUNTER_START;
  vi->bus = bus;

  if (vi_create_window(vi))
    return -1;

  gl_window_init(vi);
  return 0;
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

