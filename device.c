//
// device.c: CEN64 device container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "bus/controller.h"
#include "ai/controller.h"
#include "common.h"
#include "device.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rsp/cpu.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"

// Creates and initializes a device.
struct cen64_device *device_create(struct cen64_device *device) {
  device->ram = malloc(0x800000);

  // Initialize the bus.
  device->bus.ai = &device->ai;
  device->bus.pi = &device->pi;
  device->bus.ri = &device->ri;
  device->bus.si = &device->si;
  device->bus.vi = &device->vi;

  device->bus.rdp = &device->rdp;
  device->bus.rsp = &device->rsp;
  device->bus.vr4300 = &device->vr4300;

  // Initialize the bus.
  if (bus_init(&device->bus)) {
    printf("create_device: Failed to initialize the bus.\n");
    return NULL;
  }

  // Initialize the AI.
  if (ai_init(&device->ai, &device->bus) < 0) {
    printf("create_device: Failed to initialize the AI.\n");
    return NULL;
  }

  // Initialize the PI.
  if (pi_init(&device->pi, &device->bus, device->cart, device->cart_size) < 0) {
    printf("create_device: Failed to initialize the PI.\n");
    return NULL;
  }

  // Initialize the RI.
  if (ri_init(&device->ri, &device->bus, device->ram) < 0) {
    printf("create_device: Failed to initialize the RI.\n");
    return NULL;
  }

  // Initialize the SI.
  if (si_init(&device->si, &device->bus, device->pifrom) < 0) {
    printf("create_device: Failed to initialize the SI.\n");
    return NULL;
  }

  // Initialize the VI.
  if (vi_init(&device->vi, &device->bus) < 0) {
    printf("create_device: Failed to initialize the VI.\n");
    return NULL;
  }

  // Initialize the RDP.
  if (rdp_init(&device->rdp, &device->bus)) {
    printf("create_device: Failed to initialize the RDP.\n");
    return NULL;
  }

  // Initialize the RSP.
  if (rsp_init(&device->rsp, &device->bus)) {
    printf("create_device: Failed to initialize the RSP.\n");
    return NULL;
  }

  // Initialize the VR4300.
  if (vr4300_init(&device->vr4300, &device->bus)) {
    printf("create_device: Failed to initialize the VR4300.\n");
    return NULL;
  }

  return device;
}

// Deallocates and cleans up a device.
void device_destroy(struct cen64_device *device) {
  bus_cleanup(&device->bus);
  free(device->ram);
}

// Kicks off threads and starts the device.
int device_run(struct cen64_device *device) {
  if (setjmp(device->bus.unwind_data))
    return 0;

  while (1) {
    vi_cycle(&device->vi);
    vr4300_cycle(&device->vr4300);

    vi_cycle(&device->vi);
    vr4300_cycle(&device->vr4300);
    vr4300_cycle(&device->vr4300);
  }

  return 0;
}

