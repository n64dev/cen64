//
// rsp/opcodes.c: RSP opcode types and info.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "rsp/opcodes.h"

const char *rsp_opcode_mnemonics[NUM_RSP_OPCODES] = {
#define X(op) #op,
#include "rsp/opcodes.md"
#undef X
};

const char *rsp_vector_opcode_mnemonics[NUM_RSP_VECTOR_OPCODES] = {
#define X(op) #op,
#include "rsp/vector_opcodes.md"
#undef X
};


