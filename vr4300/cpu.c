//
// vr4300/cpu.c: VR4300 processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/cp0.h"
#include "vr4300/cpu.h"
#include "vr4300/icache.h"
#include "vr4300/pipeline.h"

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *mi_register_mnemonics[NUM_MI_REGISTERS] = {
#define X(reg) #reg,
#include "vr4300/registers.md"
#undef X
};
#endif

// Sets the opaque pointer used for external accesses.
static void vr4300_connect_bus(struct vr4300 *vr4300,
  struct bus_controller *bus) {
  vr4300->bus = bus;
}

// Initializes the VR4300 component.
int vr4300_init(struct vr4300 *vr4300, struct bus_controller *bus) {
  vr4300_connect_bus(vr4300, bus);

  vr4300_cp0_init(vr4300);
  vr4300_icache_init(&vr4300->icache);
  vr4300_pipeline_init(&vr4300->pipeline);

  vr4300->signals = VR4300_SIGNAL_COLDRESET;
  return 0;
}

