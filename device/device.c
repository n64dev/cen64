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
#include "fpu/fpu.h"
#include "os/gl_window.h"
#include "os/rom_file.h"

#include "bus/controller.h"
#include "ai/controller.h"
#include "dd/controller.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rsp/cpu.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"
#include "vr4300/cp1.h"

#ifdef CEN64_DEVFEATURES
cen64_hot static int device_spin(struct cen64_device *device,
  fpu_state_t *saved_fpu_state, struct vr4300_stats *vr4300_stats);
#else
cen64_hot static int device_spin(struct cen64_device *device);
#endif

// Creates and initializes a device.
struct cen64_device *device_create(struct cen64_device *device, uint8_t *ram,
  const struct rom_file *ddipl, const struct rom_file *ddrom,
  const struct rom_file *pifrom, const struct rom_file *cart) {

  // Initialize the bus.
  device->bus.ai = &device->ai;
  device->bus.dd = &device->dd;
  device->bus.pi = &device->pi;
  device->bus.ri = &device->ri;
  device->bus.si = &device->si;
  device->bus.vi = &device->vi;

  device->bus.rdp = &device->rdp;
  device->bus.rsp = &device->rsp;
  device->bus.vr4300 = &device->vr4300;

  // Initialize the bus.
  if (bus_init(&device->bus)) {
    debug("create_device: Failed to initialize the bus.\n");
    return NULL;
  }

  // Initialize the AI.
  if (ai_init(&device->ai, &device->bus)) {
    debug("create_device: Failed to initialize the AI.\n");
    return NULL;
  }

  // Initialize the DD.
  if (dd_init(&device->dd, &device->bus,
    ddipl->ptr, ddrom->ptr, ddrom->size)) {
    debug("create_device: Failed to initialize the DD.\n");
    return NULL;
  }

  // Initialize the PI.
  if (pi_init(&device->pi, &device->bus, cart->ptr, cart->size)) {
    debug("create_device: Failed to initialize the PI.\n");
    return NULL;
  }

  // Initialize the RI.
  if (ri_init(&device->ri, &device->bus, ram)) {
    debug("create_device: Failed to initialize the RI.\n");
    return NULL;
  }

  // Initialize the SI.
  if (si_init(&device->si, &device->bus, pifrom->ptr,
    cart->ptr, ddipl->ptr != NULL)) {
    debug("create_device: Failed to initialize the SI.\n");
    return NULL;
  }

  // Initialize the VI.
  if (vi_init(&device->vi, &device->bus)) {
    debug("create_device: Failed to initialize the VI.\n");
    return NULL;
  }

  // Initialize the RDP.
  if (rdp_init(&device->rdp, &device->bus)) {
    debug("create_device: Failed to initialize the RDP.\n");
    return NULL;
  }

  // Initialize the RSP.
  if (rsp_init(&device->rsp, &device->bus)) {
    debug("create_device: Failed to initialize the RSP.\n");
    return NULL;
  }

  // Initialize the VR4300.
  if (vr4300_init(&device->vr4300, &device->bus)) {
    debug("create_device: Failed to initialize the VR4300.\n");
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
void device_exit(struct bus_controller *bus) {
  longjmp(bus->unwind_data, 1);
}

// Create a device and proceed to the main loop.
void device_run(struct cen64_device *device) {
  struct vr4300_stats vr4300_stats;
  fpu_state_t saved_fpu_state;

  // Memory is already allocated; just spawn the device.
  memset(&vr4300_stats, 0, sizeof(vr4300_stats));

  // TODO: Preserve host registers pinned to the device.
  saved_fpu_state = fpu_get_state();
  vr4300_cp1_init(&device->vr4300);
  rsp_late_init(&device->rsp);

  // Spin the device until we return (from setjmp).
#ifdef CEN64_DEVFEATURES
  device_spin(device, &saved_fpu_state, &vr4300_stats);
#else
  device_spin(device);
#endif

  // Finalize simulation, release memory, etc.
#ifdef CEN64_DEVFEATURES
  //if (options->print_sim_stats)
  //  vr4300_print_summary(&vr4300_stats);
#endif
}

// Continually cycles the device until setjmp returns.
#ifdef CEN64_DEVFEATURES
int device_spin(struct cen64_device *device,
  fpu_state_t *saved_fpu_state, struct vr4300_stats *vr4300_stats) {
  if (setjmp(device->bus.unwind_data))
    return 1;

  while (1) {
    unsigned i;

    for (i = 0; i < 2; i++) {
      vi_cycle(&device->vi);

      rsp_cycle(&device->rsp);
      vr4300_cycle(&device->vr4300);

      // Perform additional simulation tasks NOW.
      // Make sure to preserve the FPU state.
      vr4300_cycle_extra(&device->vr4300, vr4300_stats);
    }

    vr4300_cycle(&device->vr4300);

    // Perform additional simulation tasks NOW.
    // Make sure to preserve the FPU state.
    vr4300_cycle_extra(&device->vr4300, vr4300_stats);
  }
}
#else

// Continually cycles the device until setjmp returns.
int device_spin(struct cen64_device *device) {
  if (setjmp(device->bus.unwind_data))
    return 1;

  while (1) {
    unsigned i;

    for (i = 0; i < 2; i++) {
      vr4300_cycle(&device->vr4300);
      rsp_cycle(&device->rsp);
      vi_cycle(&device->vi);

    }

    vr4300_cycle(&device->vr4300);
  }

  return 0;
}
#endif

