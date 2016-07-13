//
// rdp/cpu.c: RDP processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rdp/cpu.h"
#include "thread.h"

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *dp_register_mnemonics[NUM_DP_REGISTERS] = {
#define X(reg) #reg,
#include "rdp/registers.md"
#undef X
};
#endif

// Sets the opaque pointer used for external accesses.
static void rdp_connect_bus(struct rdp *rdp, struct bus_controller *bus) {
  rdp->bus = bus;
}

CEN64_THREAD_RETURN_TYPE rdp_thread(void *);

// Initializes the RDP component.
int rdp_init(struct rdp *rdp, struct bus_controller *bus) {
  rdp_connect_bus(rdp, bus);

  rdp->regs[DPC_STATUS_REG] = 0x80;
  cen64_mutex_create(&rdp->rdp_mutex);
  cen64_cv_create(&rdp->rdp_signal);

  return cen64_thread_create(&rdp->rdp_thread, rdp_thread, rdp);
}

