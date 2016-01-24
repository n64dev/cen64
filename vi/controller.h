//
// vi/controller.h: Video interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_VI_CONTROLLER_H
#define CEN64_VI_CONTROLLER_H
#include "common.h"
#include "gl_common.h"
#include "gl_context.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "gl_window.h"
#include "timer.h"

struct bus_controller *bus;

enum vi_register {
#define X(reg) reg,
#include "vi/registers.md"
#undef X
  NUM_VI_REGISTERS
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *vi_register_mnemonics[NUM_VI_REGISTERS];
#endif

struct render_area {
  struct {
    unsigned start;
    unsigned end;
  } x;

  struct {
    unsigned start;
    unsigned end;
  } y;

  unsigned height;
  unsigned width;
  int hskip;
};

struct vi_controller {
  struct bus_controller *bus;
  uint32_t regs[NUM_VI_REGISTERS];

  uint32_t counter;

  // Client rendering structures.
  cen64_gl_display display;
  cen64_gl_screen screen;
  cen64_gl_window window;
  cen64_gl_context context;

  struct render_area render_area;
  float viuv[8];
  float quad[8];

  cen64_time last_update_time;
  unsigned frame_count;
};

cen64_cold int vi_init(struct vi_controller *vi, struct bus_controller *bus,
  bool no_interface);

cen64_flatten cen64_hot void vi_cycle(struct vi_controller *vi);

cen64_cold int read_vi_regs(void *opaque, uint32_t address, uint32_t *word);
cen64_cold int write_vi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

