//
// device.c: CEN64 device container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <setjmp.h>
#include <stddef.h>
#include <stdlib.h>
#include "common.h"
#include "device.h"
#include "os/rom_file.h"

#include "bus/controller.h"
#include "ai/controller.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rsp/cpu.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"

cen64_cold static void device_destroy(struct cen64_device *device);
cen64_cold static struct cen64_device *device_create(struct cen64_device *device,
  uint8_t *ram, const struct rom_file *pifrom, const struct rom_file *cart);

static int device_runmode_fast(struct cen64_device *device);
static int device_runmode_extra(struct cen64_device *device);

// Creates and initializes a device.
struct cen64_device *device_create(struct cen64_device *device,
  uint8_t *ram, const struct rom_file *pifrom, const struct rom_file *cart) {

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
  if (pi_init(&device->pi, &device->bus, cart->ptr, cart->size) < 0) {
    printf("create_device: Failed to initialize the PI.\n");
    return NULL;
  }

  // Initialize the RI.
  if (ri_init(&device->ri, &device->bus, ram) < 0) {
    printf("create_device: Failed to initialize the RI.\n");
    return NULL;
  }

  // Initialize the SI.
  if (si_init(&device->si, &device->bus, pifrom->ptr) < 0) {
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

// Cleans up memory allocated for the device.
void device_destroy(struct cen64_device *device) {
  rsp_destroy(&device->rsp);
}

// Called when we should (probably?) leave simulation.
// After calling this function, we return to device_runmode_*.
void device_request_exit(struct bus_controller *bus) {
  longjmp(bus->unwind_data, 1);
}

// Kicks off threads and starts the device.
static int device_runmode_fast(struct cen64_device *device) {
  rsp_vect_t acc_lo, acc_md, acc_hi;
  struct rsp *rsp = &device->rsp;

  unsigned i;

  // Preserve host registers pinned to RSP accumulators.
  // On many hosts, these will have no real effect.
  read_acc_lo(rsp->cp2.acc, &acc_lo);
  read_acc_md(rsp->cp2.acc, &acc_md);
  read_acc_hi(rsp->cp2.acc, &acc_hi);

  write_acc_lo(rsp->cp2.acc, rsp_vzero());
  write_acc_md(rsp->cp2.acc, rsp_vzero());
  write_acc_hi(rsp->cp2.acc, rsp_vzero());

  if (setjmp(device->bus.unwind_data)) {
    read_acc_lo(rsp->cp2.acc, &acc_lo);
    read_acc_md(rsp->cp2.acc, &acc_md);
    read_acc_hi(rsp->cp2.acc, &acc_hi);

    return 0;
  }

  while (1) {
    for (i = 0; i < 2; i++) {
      vi_cycle(&device->vi);

      rsp_cycle(rsp);
      vr4300_cycle(&device->vr4300);
    }

    vr4300_cycle(&device->vr4300);
  }

  return 0;
}

// Kicks off threads and starts the device.
static int device_runmode_extra(struct cen64_device *device) {
  rsp_vect_t acc_lo, acc_md, acc_hi;
  struct rsp *rsp = &device->rsp;

  struct vr4300_stats vr4300_stats;
  unsigned i;

  memset(&vr4300_stats, 0, sizeof(vr4300_stats));

  // Preserve host registers pinned to RSP accumulators.
  // On many hosts, these will have no real effect.
  read_acc_lo(rsp->cp2.acc, &acc_lo);
  read_acc_md(rsp->cp2.acc, &acc_md);
  read_acc_hi(rsp->cp2.acc, &acc_hi);

  write_acc_lo(rsp->cp2.acc, rsp_vzero());
  write_acc_md(rsp->cp2.acc, rsp_vzero());
  write_acc_hi(rsp->cp2.acc, rsp_vzero());

  if (setjmp(device->bus.unwind_data)) {
    read_acc_lo(rsp->cp2.acc, &acc_lo);
    read_acc_md(rsp->cp2.acc, &acc_md);
    read_acc_hi(rsp->cp2.acc, &acc_hi);

    vr4300_print_summary(&vr4300_stats);
    return 0;
  }

  while (1) {
    for (i = 0; i < 2; i++) {
      vi_cycle(&device->vi);

      rsp_cycle(rsp);
      vr4300_cycle(&device->vr4300);
      vr4300_cycle_extra(&device->vr4300, &vr4300_stats);
    }

    vr4300_cycle(&device->vr4300);
    vr4300_cycle_extra(&device->vr4300, &vr4300_stats);
  }

  return 0;
}

// Create a device and proceed to the main loop.
int device_run(struct cen64_device *device, struct cen64_options *options,
  uint8_t *ram, const struct rom_file *pifrom, const struct rom_file *cart) {
  int status = EXIT_FAILURE;

  // Memory is already allocated; just spawn the device.
  if (device_create(device, ram, pifrom, cart) != NULL) {
    status = unlikely(options->extra_mode)
      ? device_runmode_extra(device)
      : device_runmode_fast(device);

    device_destroy(device);
  }

  return status;
}

