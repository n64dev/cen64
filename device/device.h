//
// device.h: Common CEN64 device container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __device_h__
#define __device_h__
#include "common.h"
#include "options.h"
#include "os/rom_file.h"

#include "ai/controller.h"
#include "bus/controller.h"
#include "dd/controller.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rdp/cpu.h"
#include "rsp/cpu.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"

#define DEVICE_RAMSIZE 0x800000U

// Only used when passed -nointerface.
extern bool device_exit_requested;

struct cen64_device {
  struct bus_controller bus;
  struct vr4300 vr4300;

  struct ai_controller ai;
  struct dd_controller dd;
  struct pi_controller pi;
  struct ri_controller ri;
  struct si_controller si;
  struct vi_controller vi;

  struct rdp rdp;
  struct rsp rsp;
};

cen64_cold void device_destroy(struct cen64_device *device);
cen64_cold struct cen64_device *device_create(struct cen64_device *device,
  uint8_t *ram, const struct rom_file *ddipl, const struct rom_file *ddrom,
  const struct rom_file *pifrom, const struct rom_file *cart);

cen64_cold void device_exit(struct bus_controller *bus);
cen64_cold void device_run(struct cen64_device *device);

#endif

