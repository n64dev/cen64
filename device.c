//
// device.c: CEN64 device container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device.h"
#include "pif/controller.h"

// Loads the PIFROM from a file into memory.
static int load_pifrom(const char *file, uint8_t *rom) {
  int status = 0;
  size_t i, last;
  FILE *f;

  if ((f = fopen(file, "rb")) == NULL) {
    printf("load_pifrom: Failed to open: %s\n", file);
    return -1;
  }

  for (i = 0; i < PIFROM_SIZE; i += last) {
    last = fread(rom + i, 1, PIFROM_SIZE - i, f);

    if (feof(f)) {
      printf("load_pifrom: ROM file is smaller than expected.\n");
      status = -1;
      break;
    }

    else if (ferror(f)) {
      printf("load_pifrom: An error occured while reading the ROM.\n");
      status = -2;
      break;
    }
  }

  fclose(f);
  return status;
}

// Creates and initializes a device.
struct cen64_device *device_create(const char *pifrom) {
  struct cen64_device *device;

  // Allocate memory.
  if ((device = (struct cen64_device*) malloc(sizeof(*device))) == NULL)
    return NULL;

  // Initialize the PIF.
  if (load_pifrom(pifrom, device->pifrom) < 0 ||
    init_pif(&device->pif, &device->bus, device->pifrom)) {
    printf("create_device: Failed to initialize the PIF.\n");

    free(device);
    return NULL;
  }

  // Initialize the bus.
  if (bus_init(&device->bus)) {
    printf("create_device: Failed to initialize the bus.\n");

    free(device);
    return NULL;
  }

  device->bus.pif = &device->pif;
  device->bus.vr4300 = &device->vr4300;

  // Initialize the VR4300.
  if (vr4300_init(&device->vr4300, &device->bus)) {
    printf("create_device: Failed to initialize the VR4300.\n");

    free(device);
    return NULL;
  }

  return device;
}

// Deallocates and cleans up a device.
void device_destroy(struct cen64_device *device) {
  free(device);
}

// Kicks off threads and starts the device.
void device_run(struct cen64_device *device) {
  unsigned i;

  for (i = 0; i < 93750000; i++)
    vr4300_cycle(&device->vr4300);
}

