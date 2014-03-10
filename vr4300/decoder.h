//
// vr4300/decoder.h: VR4300 decoder.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_decoder_h__
#define __vr4300_decoder_h__
#include "common.h"
#include "vr4300/opcodes.h"

#define GET_RS(opcode) ((opcode) >> 21 & 0x1F)
#define GET_RT(opcode) ((opcode) >> 16 & 0x1F)
#define GET_RD(opcode) ((opcode) >> 11 & 0x1F)

#define GET_FS(opcode) ((opcode) >> 11 & 0x1F)
#define GET_FT(opcode) ((opcode) >> 16 & 0x1F)
#define GET_FD(opcode) ((opcode) >>  6 & 0x1F)

#define OPCODE_INFO_NONE (0)
#define OPCODE_INFO_BRANCH (1 << 1)
#define OPCODE_INFO_NEEDRS (1 << 2)
#define OPCODE_INFO_NEEDRT (1 << 3)

struct vr4300_opcode {
  uint32_t id;
  uint32_t flags;
};

const struct vr4300_opcode* vr4300_decode_instruction(uint32_t);

#endif

