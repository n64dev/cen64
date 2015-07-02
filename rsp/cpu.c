//
// rsp/cpu.c: RSP processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rsp/cpu.h"
#include "rsp/cp0.h"

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *sp_register_mnemonics[NUM_SP_REGISTERS] = {
#define X(reg) #reg,
#include "rsp/registers.md"
#undef X
};
#endif

// Sets the opaque pointer used for external accesses.
static void rsp_connect_bus(struct rsp *rsp, struct bus_controller *bus) {
  rsp->bus = bus;
}

// Releases memory acquired for the RSP component.
void rsp_destroy(struct rsp *rsp) {
  arch_rsp_destroy(rsp);
}

// Initializes the RSP component.
int rsp_init(struct rsp *rsp, struct bus_controller *bus) {
  rsp_connect_bus(rsp, bus);

  rsp_cp0_init(rsp);
  rsp_pipeline_init(&rsp->pipeline);

  return arch_rsp_init(rsp);
}

// Initializes (host) registers.
void rsp_late_init(struct rsp *rsp) {
  write_acc_lo(rsp->cp2.acc.e, rsp_vzero());
  write_acc_md(rsp->cp2.acc.e, rsp_vzero());
  write_acc_hi(rsp->cp2.acc.e, rsp_vzero());

  write_vcc_lo(rsp->cp2.flags[RSP_VCC].e, rsp_vzero());
  write_vcc_hi(rsp->cp2.flags[RSP_VCC].e, rsp_vzero());
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, rsp_vzero());
  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, rsp_vzero());
  write_vce   (rsp->cp2.flags[RSP_VCE].e, rsp_vzero());
}
