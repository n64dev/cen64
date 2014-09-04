//
// rsp/cpu.h: RSP processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __rsp_cpu_h__
#define __rsp_cpu_h__
#include "common.h"
#include "rsp/pipeline.h"

enum rsp_register {
  RSP_REGISTER_R0, RSP_REGISTER_AT, RSP_REGISTER_V0,
  RSP_REGISTER_V1, RSP_REGISTER_A0, RSP_REGISTER_A1,
  RSP_REGISTER_A2, RSP_REGISTER_A3, RSP_REGISTER_T0,
  RSP_REGISTER_T1, RSP_REGISTER_T2, RSP_REGISTER_T3,
  RSP_REGISTER_T4, RSP_REGISTER_R5, RSP_REGISTER_T6,
  RSP_REGISTER_T7, RSP_REGISTER_S0, RSP_REGISTER_S1,
  RSP_REGISTER_S2, RSP_REGISTER_S3, RSP_REGISTER_S4,
  RSP_REGISTER_S5, RSP_REGISTER_S6, RSP_REGISTER_S7,
  RSP_REGISTER_T8, RSP_REGISTER_T9, RSP_REGISTER_K0,
  RSP_REGISTER_K1, RSP_REGISTER_GP, RSP_REGISTER_SP,
  RSP_REGISTER_FP, RSP_REGISTER_RA,

  // CP0 registers.
  RSP_REGISTER_CP0_0, RSP_REGISTER_CP0_1, RSP_REGISTER_CP0_2,
  RSP_REGISTER_CP0_3, RSP_REGISTER_CP0_4, RSP_REGISTER_CP0_5,
  RSP_REGISTER_CP0_6, RSP_REGISTER_CP0_7, RSP_REGISTER_CP0_8,
  RSP_REGISTER_CP0_9, RSP_REGISTER_CP0_10, RSP_REGISTER_CP0_11,
  RSP_REGISTER_CP0_12, RSP_REGISTER_CP0_13, RSP_REGISTER_CP0_14,
  RSP_REGISTER_CP0_15, RSP_REGISTER_CP0_16, RSP_REGISTER_CP0_17,
  RSP_REGISTER_CP0_18, RSP_REGISTER_CP0_19, RSP_REGISTER_CP0_20,
  RSP_REGISTER_CP0_21, RSP_REGISTER_CP0_22, RSP_REGISTER_CP0_23,
  RSP_REGISTER_CP0_24, RSP_REGISTER_CP0_25, RSP_REGISTER_CP0_26,
  RSP_REGISTER_CP0_27, RSP_REGISTER_CP0_28, RSP_REGISTER_CP0_29,
  RSP_REGISTER_CP0_30, RSP_REGISTER_CP0_31,

  // Miscellanious registers.
  NUM_RSP_REGISTERS
};

enum sp_register {
#define X(reg) reg,
#include "rsp/registers.md"
#undef X
  NUM_SP_REGISTERS,
  SP_REGISTER_OFFSET = RSP_REGISTER_CP0_0
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *sp_register_mnemonics[NUM_SP_REGISTERS];
#endif

struct rsp {
  struct rsp_pipeline pipeline;

  uint32_t regs[NUM_RSP_REGISTERS];
  uint8_t mem[0x2000];

  struct bus_controller *bus;
};

void rsp_cycle(struct rsp *rsp);
int rsp_init(struct rsp *rsp, struct bus_controller *bus);

#endif

