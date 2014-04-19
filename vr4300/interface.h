//
// vr4300/interface.h: MI interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_interface_h__
#define __vr4300_interface_h__
#include "common.h"
#include "vr4300/cpu.h"

enum rcp_interrupt_mask {
  MI_INTR_SP = 0x01,
  MI_INTR_SI = 0x02,
  MI_INTR_AI = 0x04,
  MI_INTR_VI = 0x08,
  MI_INTR_PI = 0x10,
  MI_INTR_DP = 0x20
};

int read_mi_regs(void *opaque, uint32_t address, uint32_t *word);
int write_mi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

void clear_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask);
void signal_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask);

#endif

