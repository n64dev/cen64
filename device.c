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

// Loads the cart from a file into memory.
static int load_cart(const char *file, uint8_t *rom) {
  int status = 0;
  size_t i, last;
  long int size;
  FILE *f;

  if ((f = fopen(file, "rb")) == NULL) {
    printf("load_cart: Failed to open: %s\n", file);
    return -1;
  }

  if (fseek(f, 0, SEEK_END) == -1 || (size = ftell(f)) == -1) {
    printf("load_cart: Failed to determine ROM size.");

    fclose(f);
    return -2;
  }

  rewind(f);

  for (i = 0; i < (unsigned long int) size; i += last) {
    last = fread(rom + i, 1, size - i, f);

    if (feof(f)) {
      printf("load_cart: ROM file is smaller than expected.\n");
      status = -3;
      break;
    }

    else if (ferror(f)) {
      printf("load_cart: An error occured while reading the ROM.\n");
      status = -4;
      break;
    }
  }

  fclose(f);
  return status;
}

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
struct cen64_device *device_create(struct cen64_device *device,
  const char *pifrom, const char *rom) {
  device->rom = malloc(0x2000000);
  device->ram = malloc(0x800000);

  // Read the PIFROM into the device.
  if (load_pifrom(pifrom, device->pifrom) < 0) {
    printf("create_device: Failed to load PIFROM.\n");
    return NULL;
  }

  // Read the ROM into the device.
  if (load_cart(rom, device->rom) < 0) {
    printf("create_device: Failed to load cart.\n");
    return NULL;
  }

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
  if (pi_init(&device->pi, &device->bus, device->rom) < 0) {
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
}

// Kicks off threads and starts the device.
void device_run(struct cen64_device *device) {
  while (1) {
    vi_cycle(&device->vi);
    vr4300_cycle(&device->vr4300);
  }
}

