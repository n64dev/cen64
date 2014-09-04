//
// vr4300/opcodes.h: VR4300 opcode types and info.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_opcodes_h__
#define __vr4300_opcodes_h__
#include "common.h"

enum vr4300_opcode_id {
#define X(op) VR4300_OPCODE_##op,
#include "vr4300/opcodes.md"
  NUM_VR4300_OPCODES
#undef X
};

struct vr4300;
typedef int (*const vr4300_function)(struct vr4300 *,
  uint32_t, uint64_t, uint64_t);

extern const vr4300_function vr4300_function_table[NUM_VR4300_OPCODES];
extern const char *vr4300_opcode_mnemonics[NUM_VR4300_OPCODES];

#endif

