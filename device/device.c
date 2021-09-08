//
// device.c: CEN64 device container.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device/device.h"
#include "device/netapi.h"
#include "fpu/fpu.h"
#include "gl_window.h"
#include "os/common/rom_file.h"
#include "os/common/save_file.h"

#include "bus/controller.h"
#include "ai/controller.h"
#include "dd/controller.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rsp/cpu.h"
#include "rsp/interface.h"
#include "thread.h"
#include "vi/controller.h"
#include "vr4300/interface.h"
#include <setjmp.h>
#include <limits.h>

cen64_cold int angrylion_rdp_init(struct cen64_device *device);
cen64_cold static int device_debug_spin(struct cen64_device *device);
cen64_cold static int device_multithread_spin(struct cen64_device *device);
cen64_flatten cen64_hot static int device_spin(struct cen64_device *device);

cen64_flatten cen64_hot static CEN64_THREAD_RETURN_TYPE run_rcp_thread(void *);
cen64_flatten cen64_hot static CEN64_THREAD_RETURN_TYPE run_vr4300_thread(void *);

// Creates and initializes a device.
struct cen64_device *device_create(struct cen64_device *device,
  const struct rom_file *ddipl, const struct dd_variant *dd_variant,
  const struct rom_file *ddrom,
  const struct rom_file *pifrom, const struct rom_file *cart,
  const struct save_file *eeprom, const struct save_file *sram,
  const struct save_file *flashram, struct is_viewer *is,
  const struct controller *controller,
  bool no_audio, bool no_video, bool profiling) {

  // Allocate memory for VR4300
  if ((device->vr4300 = vr4300_alloc()) == NULL) {
    debug("create_device: Failed to allocate memory for the VR4300.\n");
    return NULL;
  }

  // Initialize the bus.
  device->bus.ai = &device->ai;
  device->bus.dd = &device->dd;
  device->bus.pi = &device->pi;
  device->bus.ri = &device->ri;
  device->bus.si = &device->si;
  device->bus.vi = &device->vi;

  device->bus.rdp = &device->rdp;
  device->bus.rsp = &device->rsp;
  device->bus.vr4300 = device->vr4300;

  // Initialize the bus.
  if (bus_init(&device->bus, dd_variant != NULL)) {
    debug("create_device: Failed to initialize the bus.\n");
    return NULL;
  }

  // Initialize the AI.
  if (ai_init(&device->ai, &device->bus, no_audio)) {
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
  if (pi_init(&device->pi, &device->bus, cart->ptr, cart->size, sram, flashram, is)) {
    debug("create_device: Failed to initialize the PI.\n");
    return NULL;
  }

  // Initialize the RI.
  if (ri_init(&device->ri, &device->bus)) {
    debug("create_device: Failed to initialize the RI.\n");
    return NULL;
  }

  // Initialize the SI.
  if (si_init(&device->si, &device->bus, pifrom->ptr,
    cart->ptr, dd_variant, eeprom->ptr, eeprom->size,
    controller)) {
    debug("create_device: Failed to initialize the SI.\n");
    return NULL;
  }

  // Initialize the VI.
  if (vi_init(&device->vi, &device->bus, no_video)) {
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
  if (vr4300_init(device->vr4300, &device->bus, profiling)) {
    debug("create_device: Failed to initialize the VR4300.\n");
    return NULL;
  }

  angrylion_rdp_init(device);
  return device;
}

// Cleans up memory allocated for the device.
void device_destroy(struct cen64_device *device, const char *cart_path) {
  vr4300_free(device->vr4300);
  rsp_destroy(&device->rsp);

  // Save profiling data, if any
  if (cart_path && has_profile_samples(device->vr4300)) {
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s.profile", cart_path);
    path[PATH_MAX - 1] = '\0';

    FILE *f = fopen(path, "w");
    if (!f) {
      printf("Can't open %s\n", path);
      return;
    }

    uint32_t i;
    for (i = 0; i < 8 * 1024 * 1024; i++) {
      const uint64_t sample = get_profile_sample(device->vr4300, i);
      const uint64_t l1d_sample = get_profile_sample(device->vr4300, i + (8 * 1024 * 1024));
      if (sample < 10 && l1d_sample < 10)
        continue;
      fprintf(f, "%x %lu %lu\n", i + 0x80000000, sample, l1d_sample);
    }

    fclose(f);
  }
}

// Called when we should (probably?) leave simulation.
// After calling this function, we return to device_runmode_*.
void device_exit(struct bus_controller *bus) {
  longjmp(bus->unwind_data, 1);
}

// Create a device and proceed to the main loop.
void device_run(struct cen64_device *device) {
  fpu_state_t saved_fpu_state;

  // TODO: Preserve host registers pinned to the device.
  saved_fpu_state = fpu_get_state();
  vr4300_cp1_init(device->vr4300);
  rsp_late_init(&device->rsp);

  // Spin the device until we return (from setjmp).
  if (unlikely(device->debug_sfd > 0))
    device_debug_spin(device);

  else if (device->multithread)
    device_multithread_spin(device);

  else
    device_spin(device);

  // TODO: Restore host registers that were pinned.
  fpu_set_state(saved_fpu_state);
}

CEN64_THREAD_RETURN_TYPE run_rcp_thread(void *opaque) {
  struct cen64_device *device = (struct cen64_device *) opaque;

  if (setjmp(device->bus.unwind_data))
    return CEN64_THREAD_RETURN_VAL;

  while (likely(device->running)) {
    unsigned i;

    for (i = 0; i < 6250; i++) {
      rsp_cycle(&device->rsp);
      vi_cycle(&device->vi);
    }

    // Sync up with the VR4300 thread.
    cen64_mutex_lock(&device->sync_mutex);

    if (!device->other_thread_is_waiting) {
      device->other_thread_is_waiting = true;
      cen64_cv_wait(&device->sync_cv, &device->sync_mutex);
    }

    else {
      device->other_thread_is_waiting = false;
      cen64_mutex_unlock(&device->sync_mutex);
      cen64_cv_signal(&device->sync_cv);
    }
  }

  return CEN64_THREAD_RETURN_VAL;
}

CEN64_THREAD_RETURN_TYPE run_vr4300_thread(void *opaque) {
  cen64_thread_setname(NULL, "vr4300");
  struct cen64_device *device = (struct cen64_device *) opaque;

  while (likely(device->running)) {
    unsigned i, j;

    for (i = 0; i < 6250 / 2; i++) {
      for (j = 0; j < 2; j++) {
        ai_cycle(&device->ai);
        pi_cycle(&device->pi);
      }

      for (j = 0; j < 3; j++)
        vr4300_cycle(device->vr4300);
    }

    // Sync up with the RCP thread.
    cen64_mutex_lock(&device->sync_mutex);

    if (!device->other_thread_is_waiting) {
      device->other_thread_is_waiting = true;
      cen64_cv_wait(&device->sync_cv, &device->sync_mutex);
    }

    else {
      device->other_thread_is_waiting = false;
      cen64_mutex_unlock(&device->sync_mutex);
      cen64_cv_signal(&device->sync_cv);
    }
  }

  return CEN64_THREAD_RETURN_VAL;
}

// Continually cycles the device until setjmp returns.
int device_multithread_spin(struct cen64_device *device) {
  cen64_thread vr4300_thread;

  device->other_thread_is_waiting = false;

  if (cen64_mutex_create(&device->sync_mutex)) {
    printf("Failed to create the synchronization mutex.\n");
    return 1;
  }

  if (cen64_cv_create(&device->sync_cv)) {
    printf("Failed to create the synchronization CV.\n");
    cen64_mutex_destroy(&device->sync_mutex);
    return 1;
  }

  if (cen64_thread_create(&vr4300_thread, run_vr4300_thread, device)) {
    printf("Failed to create the VR4300 thread.\n");
    cen64_cv_destroy(&device->sync_cv);
    cen64_mutex_destroy(&device->sync_mutex);
    return 1;
  }


  cen64_thread_setname(vr4300_thread, "vr4300");

  run_rcp_thread(device);

  cen64_thread_join(&vr4300_thread);
  cen64_cv_destroy(&device->sync_cv);
  cen64_mutex_destroy(&device->sync_mutex);
  return 0;
}

// Continually cycles the device until setjmp returns.
int device_spin(struct cen64_device *device) {
  if (setjmp(device->bus.unwind_data))
    return 1;

  while (likely(device->running)) {
    unsigned i;

    for (i = 0; i < 2; i++) {
      vr4300_cycle(device->vr4300);
      rsp_cycle(&device->rsp);
      ai_cycle(&device->ai);
      pi_cycle(&device->pi);
      vi_cycle(&device->vi);

    }

    vr4300_cycle(device->vr4300);
  }

  return 0;
}

// Continually cycles the device until setjmp returns.
int device_debug_spin(struct cen64_device *device) {
  struct vr4300_stats* vr4300_stats = vr4300_stats_alloc();

  // Prepare stats, set a breakpoint @ VR4300 IPL vector.

  netapi_debug_wait(device->debug_sfd, device);

  if (setjmp(device->bus.unwind_data))
    return 1;

  while (likely(device->running)) {
    unsigned i;

    for (i = 0; i < 2; i++) {
      vr4300_cycle(device->vr4300);
      rsp_cycle(&device->rsp);
      ai_cycle(&device->ai);
      pi_cycle(&device->pi);
      vi_cycle(&device->vi);

      vr4300_cycle_extra(device->vr4300, vr4300_stats);

    }

    vr4300_cycle(device->vr4300);
    vr4300_cycle_extra(device->vr4300, vr4300_stats);
  }

  vr4300_stats_free(vr4300_stats);
  return 0;
}

cen64_cold void device_connect_debugger(struct cen64_device *device, void* break_handler_data, debug_break_handler break_handler) {
  vr4300_connect_debugger(device->vr4300, break_handler_data, break_handler);
  rsp_connect_debugger(&device->rsp, break_handler_data, break_handler);
}