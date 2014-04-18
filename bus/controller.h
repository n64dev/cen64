//
// bus/controller.h: System bus controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __bus_controller_h__
#define __bus_controller_h__
#include "common.h"
#include "bus/memorymap.h"

struct ai_controller;
struct pi_controller;
struct ri_controller;
struct si_controller;
struct vi_controller;

struct rdp;
struct rsp;
struct vr4300;

struct bus_controller {
  struct memory_map *map;

  struct ai_controller *ai;
  struct pi_controller *pi;
  struct ri_controller *ri;
  struct si_controller *si;
  struct vi_controller *vi;

  struct rdp *rdp;
  struct rsp *rsp;
  struct vr4300 *vr4300;
};

int bus_init(struct bus_controller *bus);

// General-purpose accesssor functions.
int bus_read_word(struct bus_controller *bus,
  uint32_t address, uint32_t *word);

int bus_write_word(struct bus_controller *bus,
  uint32_t address, uint32_t word, uint32_t dqm);

// For asserting and deasserting RCP interrupts.
enum rcp_interrupt_mask;

int raise_rcp_interrupt(struct bus_controller *bus,
  enum rcp_interrupt_mask mask);

#endif

