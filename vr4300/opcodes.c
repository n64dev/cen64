//
// vr4300/opcodes.c: VR4300 opcode types and info.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "vr4300/opcodes.h"

const char *vr4300_opcode_mnemonics[NUM_VR4300_OPCODES] = {
#define X(op) #op,
#include "vr4300/opcodes.md"
#undef X
};

