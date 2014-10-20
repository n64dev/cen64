//
// vr4300/fault.h: VR4300 fault management.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_fault_h__
#define __vr4300_fault_h__

enum vr4300_fault_id {
#define X(fault) VR4300_FAULT_##fault,
#include "vr4300/fault.md"
  NUM_VR4300_FAULTS
#undef X
};

extern const char *vr4300_fault_mnemonics[NUM_VR4300_FAULTS];

#define X(fault) void VR4300_##fault(struct vr4300 *vr4300);
#include "vr4300/fault.md"
#undef X

#endif

