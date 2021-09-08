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
  debug_cleanup(&rsp->debug);
}

// Initializes the RSP component.
int rsp_init(struct rsp *rsp, struct bus_controller *bus) {
  rsp_connect_bus(rsp, bus);

  rsp_cp0_init(rsp);
  rsp_pipeline_init(&rsp->pipeline);

  debug_init(&rsp->debug, DEBUG_SOURCE_RSP);

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

void rsp_signal_break(struct rsp *rsp) {
  debug_signal(&rsp->debug, DEBUG_SIGNALS_BREAK);
}

void rsp_set_breakpoint(struct rsp *rsp, uint64_t at) {
  debug_set_breakpoint(&rsp->debug, at);
}

void rsp_remove_breakpoint(struct rsp *rsp, uint64_t at) {
  debug_remove_breakpoint(&rsp->debug, at);
}

uint32_t rsp_get_register(struct rsp *rsp, size_t i) {
  return rsp->regs[i];
}

uint32_t rsp_get_pc(struct rsp *rsp) {
  return rsp->pipeline.dfwb_latch.common.pc;
}

void rsp_connect_debugger(struct rsp *rsp, void* break_handler_data, debug_break_handler break_handler) {
  rsp->debug.break_handler = break_handler;
  rsp->debug.break_handler_data = break_handler_data;
}