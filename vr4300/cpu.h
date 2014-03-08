//
// vr4300/cpu.h: VR4300 processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_cpu_h__
#define __vr4300_cpu_h__
#include "common.h"
#include "vr4300/cp0.h"
#include "vr4300/icache.h"
#include "vr4300/pipeline.h"

struct bus_controller;

enum vr4300_signals {
  VR4300_SIGNAL_FORCEEXIT = 0x000000001,
  VR4300_SIGNAL_COLDRESET = 0x000000002,
};

enum vr4300_register {
  VR4300_REGISTER_R0, VR4300_REGISTER_AT, VR4300_REGISTER_V0,
  VR4300_REGISTER_V1, VR4300_REGISTER_A0, VR4300_REGISTER_A1,
  VR4300_REGISTER_A2, VR4300_REGISTER_A3, VR4300_REGISTER_T0,
  VR4300_REGISTER_T1, VR4300_REGISTER_T2, VR4300_REGISTER_T3,
  VR4300_REGISTER_T4, VR4300_REGISTER_R5, VR4300_REGISTER_T6,
  VR4300_REGISTER_T7, VR4300_REGISTER_S0, VR4300_REGISTER_S1,
  VR4300_REGISTER_S2, VR4300_REGISTER_S3, VR4300_REGISTER_S4,
  VR4300_REGISTER_S5, VR4300_REGISTER_S6, VR4300_REGISTER_S7,
  VR4300_REGISTER_T8, VR4300_REGISTER_T9, VR4300_REGISTER_K0,
  VR4300_REGISTER_K1, VR4300_REGISTER_GP, VR4300_REGISTER_SP,
  VR4300_REGISTER_FP, VR4300_REGISTER_RA, VR4300_REGISTER_LO,
  VR4300_REGISTER_HI, NUM_VR4300_REGISTERS
};

struct vr4300 {
  uint64_t regs[NUM_VR4300_REGISTERS];

  struct vr4300_cp0 cp0;
  struct vr4300_icache icache;
  struct vr4300_pipeline pipeline;

  struct bus_controller *bus;
  unsigned signals;
};

void vr4300_cycle(struct vr4300 *vr4300);
int vr4300_init(struct vr4300 *vr4300, struct bus_controller *bus);

#endif

