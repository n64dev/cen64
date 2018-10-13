//
// bus/controller.h: System bus controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __bus_controller_h__
#define __bus_controller_h__
#include "common.h"
#include "bus/memorymap.h"
#include <setjmp.h>

struct ai_controller;
struct dd_controller;
struct pi_controller;
struct ri_controller;
struct si_controller;
struct vi_controller;

struct rdp;
struct rsp;
struct vr4300;

struct bus_controller {
  struct ai_controller *ai;
  struct dd_controller *dd;
  struct pi_controller *pi;
  struct ri_controller *ri;
  struct si_controller *si;
  struct vi_controller *vi;

  struct rdp *rdp;
  struct rsp *rsp;
  struct vr4300 *vr4300;

  // For resolving physical address ranges to devices.
  struct memory_map map;

  // Allows to to pop back out into device_run during simulation.
  // Kind of a hack to put this in with the device "bus", but at
  // least everyone gets access to it this way.
  jmp_buf unwind_data;
};

cen64_cold int bus_init(struct bus_controller *bus, int dd_present);

// General-purpose accesssor functions.
cen64_flatten cen64_hot int bus_read_word(const struct bus_controller *bus,
  uint32_t address, uint32_t *word);

cen64_flatten cen64_hot int bus_write_word(struct bus_controller *bus,
  uint32_t address, uint32_t word, uint32_t dqm);

// For asserting and deasserting RCP interrupts.
enum rcp_interrupt_mask;

int raise_rcp_interrupt(struct bus_controller *bus,
  enum rcp_interrupt_mask mask);

#endif

