/* ============================================================================
 *  CEN64.c: Main application.
 *
 *  CEN64: Cycle-accurate, Efficient Nintendo 64 Simulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "CEN64.h"
#include "Device.h"

#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#include <GL/glfw.h>

void VR4300RaiseRCPInterrupt(struct VR4300 *, unsigned);

/* ============================================================================
 *  main: Parses arguments and kicks off the application.
 * ========================================================================= */
int main(int argc, const char *argv[]) {
  struct CEN64Device *device;

  if (argc != 3) {
    printf("Usage: %s <pifrom> <cart>\n\n", argv[0]);
    printf("RSP Build Type: %s\nRDP Build Type: %s\n",
      RSPBuildType, RDPBuildType);

    return 0;
  }

  if ((device = CreateDevice(argv[1])) == NULL) {
    printf("Failed to create a device.\n");
    return 1;
  }

  if (LoadCartridge(device, argv[2])) {
    printf("Failed to load the ROM.\n");

    DestroyDevice(device);
    return 2;
  }

  debug("== Booting the Console ==");

  while (1)
    CycleDevice(device);

  DestroyDevice(device);
  return 0;
}

