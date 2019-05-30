//
// vr4300/interface.h: MI interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_interface_h__
#define __vr4300_interface_h__
#include "common.h"

enum rcp_interrupt_mask {
  MI_INTR_SP = 0x01,
  MI_INTR_SI = 0x02,
  MI_INTR_AI = 0x04,
  MI_INTR_VI = 0x08,
  MI_INTR_PI = 0x10,
  MI_INTR_DP = 0x20
};

struct vr4300;
struct vr4300_stats;

cen64_cold struct vr4300* vr4300_alloc();
cen64_cold void vr4300_free(struct vr4300*);

cen64_cold struct vr4300_stats* vr4300_stats_alloc();
cen64_cold void vr4300_stats_free(struct vr4300_stats*);
    
cen64_cold int vr4300_init(struct vr4300 *vr4300, struct bus_controller *bus, bool profiling);
cen64_cold void vr4300_cp1_init(struct vr4300 *vr4300);

cen64_flatten cen64_hot void vr4300_cycle(struct vr4300 *vr4300);
cen64_cold void vr4300_cycle_extra(struct vr4300 *vr4300, struct vr4300_stats *stats);

uint64_t vr4300_get_register(struct vr4300 *vr4300, size_t i);
uint64_t vr4300_get_pc(struct vr4300 *vr4300);

int read_mi_regs(void *opaque, uint32_t address, uint32_t *word);
int write_mi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

void clear_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask);
void signal_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask);

void clear_dd_interrupt(struct vr4300 *vr4300);
void signal_dd_interrupt(struct vr4300 *vr4300);

uint64_t get_profile_sample(struct vr4300 const *vr4300, size_t i);
int has_profile_samples(struct vr4300 const *vr4300);

#endif

