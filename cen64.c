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
#include "os/rom_file.h"
#include <setjmp.h>

static int load_roms(const char *pifrom_path, const char *cart_path,
  struct rom_file *pifrom, struct rom_file *cart);

static int setup_and_run_device(struct cen64_device *device,
  const char *pifrom_path, const char *cart_path);

// Called when a simulation instance is terminating.
void cen64_cleanup(struct cen64_device *device) {
  destroy_gl_window(&device->vi.gl_window);
  device_destroy(device);
}

// Called when another simulation instance is desired.
int cen64_main(struct cen64_device *device, int argc, const char *argv[]) {
  struct gl_window_hints hints;
  int status;

  // Prevent debugging tools from raising warnings
  // about uninitialized memory being read, etc.
  memset(device, 0, sizeof(*device));
  get_default_gl_window_hints(&hints);

  if (argc < 3) {
    printf("%s <pifrom.bin> <rom>\n", argv[0]);
    return 255;
  }

  if (create_gl_window(&device->vi.gl_window, &hints)) {
    printf("Failed to create a window.\n");
    return 1;
  }

  // Start simulation.
  if (!(status = setup_and_run_device(device, argv[1], argv[2]))) {
    destroy_gl_window(&device->vi.gl_window);
    return 2;
  }

  return status;
}

// Called to temporary (or permanently) leave simulation.
// After calling this function, we return to device_run.
void cen64_return(struct bus_controller *bus) {
  longjmp(bus->unwind_data, 1);
}

// Load any ROM images required for simulation.
int load_roms(const char *pifrom_path, const char *cart_path,
  struct rom_file *pifrom, struct rom_file *cart) {
  if (open_rom_file(pifrom_path, pifrom)) {
    printf("Failed to load PIF ROM: %s.\n", pifrom_path);

    return 1;
  }

  if (open_rom_file(cart_path, cart)) {
    printf("Failed to load cart: %s.\n", cart_path);

    close_rom_file(pifrom);
    return 2;
  }

  return 0;
}

// Create a device, Load ROM images, etc. and run.
int setup_and_run_device(struct cen64_device *device,
  const char *pifrom_path, const char *cart_path) {
  struct rom_file pifrom, cart;
  int status;

  if (!(status = load_roms(pifrom_path, cart_path, &pifrom, &cart))) {
    device->pifrom = pifrom.ptr;
    device->cart = cart.ptr;
    device->pifrom_size = pifrom.size;
    device->cart_size = cart.size;

    // Create a device and roceed to the main application loop.
    if (!(status = device_create(device) == NULL))
      status = device_run(device);

    close_rom_file(&cart);
    close_rom_file(&pifrom);
  }

  return status;
}

