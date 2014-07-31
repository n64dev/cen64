//
// cen64.c: CEN64 entry point.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device.h"
#include <setjmp.h>

// Called when a simulation instance is terminating.
void cen64_cleanup(struct cen64_device *device) {
  destroy_gl_window(&device->vi.gl_window);
  device_destroy(device);
}

// Called when another simulation instance is desired.
int cen64_main(struct cen64_device *device, int argc, const char *argv[]) {
  struct gl_window_hints hints;
  get_default_gl_window_hints(&hints);

  // Prevent debugging tools from raising warnings
  // about uninitialized memory being read, etc.
  memset(device, 0, sizeof(*device));

  if (argc < 3) {
    printf("%s <pifrom.bin> <rom>\n", argv[0]);
    return 255;
  }

  if (create_gl_window(&device->vi.gl_window, &hints)) {
    printf("Failed to create a window.\n");
    return 1;
  }

  if (device_create(device, argv[1], argv[2]) == NULL) {
    printf("Failed to create a device.\n");

    destroy_gl_window(&device->vi.gl_window);
    return 2;
  }

  device_run(device);
  return 0;
}

// Called to temporary (or permanently) leave simulation.
// After calling this function, we return to device_run.
void cen64_return(struct bus_controller *bus) {
  longjmp(bus->unwind_data, 1);
}

