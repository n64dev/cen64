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
#include "vr4300/cp1.h"
#include "vr4300/dcache.h"
#include "vr4300/icache.h"
#include "vr4300/opcodes.h"
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
  VR4300_REGISTER_FP, VR4300_REGISTER_RA,

  // CP0 registers.
  VR4300_REGISTER_CP0_0, VR4300_REGISTER_CP0_1, VR4300_REGISTER_CP0_2,
  VR4300_REGISTER_CP0_3, VR4300_REGISTER_CP0_4, VR4300_REGISTER_CP0_5,
  VR4300_REGISTER_CP0_6, VR4300_REGISTER_CP0_7, VR4300_REGISTER_CP0_8,
  VR4300_REGISTER_CP0_9, VR4300_REGISTER_CP0_10, VR4300_REGISTER_CP0_11,
  VR4300_REGISTER_CP0_12, VR4300_REGISTER_CP0_13, VR4300_REGISTER_CP0_14,
  VR4300_REGISTER_CP0_15, VR4300_REGISTER_CP0_16, VR4300_REGISTER_CP0_17,
  VR4300_REGISTER_CP0_18, VR4300_REGISTER_CP0_19, VR4300_REGISTER_CP0_20,
  VR4300_REGISTER_CP0_21, VR4300_REGISTER_CP0_22, VR4300_REGISTER_CP0_23,
  VR4300_REGISTER_CP0_24, VR4300_REGISTER_CP0_25, VR4300_REGISTER_CP0_26,
  VR4300_REGISTER_CP0_27, VR4300_REGISTER_CP0_28, VR4300_REGISTER_CP0_29,
  VR4300_REGISTER_CP0_30, VR4300_REGISTER_CP0_31,

  // CP1 registers.
  VR4300_REGISTER_CP1_0, VR4300_REGISTER_CP1_1, VR4300_REGISTER_CP1_2,
  VR4300_REGISTER_CP1_3, VR4300_REGISTER_CP1_4, VR4300_REGISTER_CP1_5,
  VR4300_REGISTER_CP1_6, VR4300_REGISTER_CP1_7, VR4300_REGISTER_CP1_8,
  VR4300_REGISTER_CP1_9, VR4300_REGISTER_CP1_10, VR4300_REGISTER_CP1_11,
  VR4300_REGISTER_CP1_12, VR4300_REGISTER_CP1_13, VR4300_REGISTER_CP1_14,
  VR4300_REGISTER_CP1_15, VR4300_REGISTER_CP1_16, VR4300_REGISTER_CP1_17,
  VR4300_REGISTER_CP1_18, VR4300_REGISTER_CP1_19, VR4300_REGISTER_CP1_20,
  VR4300_REGISTER_CP1_21, VR4300_REGISTER_CP1_22, VR4300_REGISTER_CP1_23,
  VR4300_REGISTER_CP1_24, VR4300_REGISTER_CP1_25, VR4300_REGISTER_CP1_26,
  VR4300_REGISTER_CP1_27, VR4300_REGISTER_CP1_28, VR4300_REGISTER_CP1_29,
  VR4300_REGISTER_CP1_30, VR4300_REGISTER_CP1_31,

  // Miscellanious registers.
  VR4300_REGISTER_HI, VR4300_REGISTER_LO,
  VR4300_CP1_FCR0, VR4300_CP1_FCR31,

  // Pipeline cycle type flag.
  //
  // Putting these here along with the other registers allows us to
  // correctly (and cheaply) detect that a busy-wait loop instruction
  // is retiring.
  PIPELINE_CYCLE_TYPE,

  NUM_VR4300_REGISTERS
};

enum mi_register {
#define X(reg) reg,
#include "vr4300/registers.md"
#undef X
  NUM_MI_REGISTERS,
};

#ifdef DEBUG_MMIO_REGISTER_ACCESS
extern const char *mi_register_mnemonics[NUM_MI_REGISTERS];
#endif

struct vr4300 {
  struct vr4300_pipeline pipeline;

  // Align the TLB to a 16-byte boundary for vectorization.
  // TODO: Handle the fact that MSVC doesn't like 0-sized arrays.
  //uint8_t padding_for_tlb[(16 - (sizeof(struct vr4300_pipeline) % 16)) % 16];
  uint8_t padding_for_tlb[16 - (sizeof(struct vr4300_pipeline) % 16)];
  struct vr4300_cp0 cp0;

  struct bus_controller *bus;
  unsigned signals;

  uint64_t regs[NUM_VR4300_REGISTERS];
  uint32_t mi_regs[NUM_MI_REGISTERS];

  struct vr4300_dcache dcache;
  struct vr4300_icache icache;

};

struct vr4300_stats {
  unsigned long executed_instructions;
  unsigned long total_cycles;

  unsigned long opcode_counts[NUM_VR4300_OPCODES];
};

cen64_cold int vr4300_init(struct vr4300 *vr4300, struct bus_controller *bus);
cen64_cold void vr4300_print_summary(struct vr4300_stats *stats);

cen64_hot void vr4300_cycle(struct vr4300 *vr4300);
cen64_hot void vr4300_cycle_extra(struct vr4300 *vr4300, struct vr4300_stats *stats);

#endif

