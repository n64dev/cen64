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
#include "os/main.h"
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

