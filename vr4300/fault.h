//
// vr4300/fault.h: VR4300 fault management.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_fault_h__
#define __vr4300_fault_h__

// Currently using fixed values....
#define DCACHE_ACCESS_DELAY (46 - 2)
#define ICACHE_ACCESS_DELAY (50 - 2)
#define MEMORY_WORD_DELAY 38

enum vr4300_fault_id {
#define X(fault) VR4300_FAULT_##fault,
#include "vr4300/fault.md"
  NUM_VR4300_FAULTS
#undef X
};

extern const char *vr4300_fault_mnemonics[NUM_VR4300_FAULTS];

cen64_cold void VR4300_CPU(struct vr4300 *vr4300);
cen64_cold void VR4300_DADE(struct vr4300 *vr4300);
cen64_cold void VR4300_DCB(struct vr4300 *vr4300);
cen64_cold void VR4300_DCM(struct vr4300 *vr4300);
cen64_cold void VR4300_IADE(struct vr4300 *vr4300);
cen64_cold void VR4300_ICB(struct vr4300 *vr4300);
cen64_cold void VR4300_INTR(struct vr4300 *vr4300);
cen64_cold void VR4300_INV(struct vr4300 *vr4300);
cen64_cold void VR4300_LDI(struct vr4300 *vr4300);
cen64_cold void VR4300_RST(struct vr4300 *vr4300);
cen64_cold void VR4300_WAT(struct vr4300 *vr4300);

cen64_cold void VR4300_DTLB(struct vr4300 *vr4300, unsigned miss, unsigned inv, unsigned mod);
cen64_cold void VR4300_ITLB(struct vr4300 *vr4300, unsigned miss);

#endif

