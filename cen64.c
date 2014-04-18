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

int main(int argc, const char *argv[]) {
  struct cen64_device device;

  if (argc < 3) {
    printf("%s <pifrom.bin> <rom>\n", argv[0]);
    return 0;
  }

  if (device_create(&device, argv[1], argv[2]) == NULL) {
    printf("Failed to create a device.\n");
    return 1;
  }

  device_run(&device);

  device_destroy(&device);
  return 0;
}

