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
#include "ai/controller.h"
#include "bus/controller.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rdp/cpu.h"
#include "rsp/cpu.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"

#define PIFROM_SIZE 2048

struct cen64_device {
  struct bus_controller bus;

  struct ai_controller ai;
  struct pi_controller pi;
  struct ri_controller ri;
  struct si_controller si;
  struct vi_controller vi;

  struct rdp rdp;
  struct rsp rsp;
  struct vr4300 vr4300;

  // Dynamic memory.
  uint8_t *ram;

  // Read only images.
  size_t pifrom_size;
  size_t cart_size;

  const uint8_t *pifrom;
  const uint8_t *cart;
};

struct cen64_device *device_create(struct cen64_device *device);
void device_destroy(struct cen64_device *device);
int device_run(struct cen64_device *device);

#endif

