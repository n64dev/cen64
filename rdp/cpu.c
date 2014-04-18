//
// rdp/cpu.c: RDP processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rdp/cpu.h"

// Sets the opaque pointer used for external accesses.
static void rdp_connect_bus(struct rdp *rdp, struct bus_controller *bus) {
  rdp->bus = bus;
}

// Initializes the RDP component.
int rdp_init(struct rdp *rdp, struct bus_controller *bus) {
  rdp_connect_bus(rdp, bus);

  return 0;
}

