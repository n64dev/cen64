//
// device.h: CEN64 device container.
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
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rdp/cpu.h"
#include "rsp/cpu.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"

#define DEVICE_RAMSIZE 0x800000U

struct cen64_device {
  struct bus_controller bus;
  uint8_t padding[32];
  struct vr4300 vr4300;

  struct ai_controller ai;
  struct pi_controller pi;
  struct ri_controller ri;
  struct si_controller si;
  struct vi_controller vi;

  struct rdp rdp;
  struct rsp rsp;
};

cen64_cold void device_request_exit(struct bus_controller *bus);

cen64_hot int device_run(struct cen64_device *device, struct cen64_options *options,
  uint8_t *ram, const struct rom_file *pifrom, const struct rom_file *cart);

#endif

