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
#include "bus/controller.h"
#include "pif/controller.h"
#include "vr4300/cpu.h"

#define PIFROM_SIZE 2048

struct cen64_device {
  struct bus_controller bus;
  struct pif_controller pif;
  struct vr4300 vr4300;

  uint8_t pifrom[PIFROM_SIZE];
};

struct cen64_device *device_create(const char *pifrom);
void device_destroy(struct cen64_device *device);

void device_run(struct cen64_device *device);

#endif

