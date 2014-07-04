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

#define GET_RS(iw) ((iw) >> 21 & 0x1F)
#define GET_RT(iw) ((iw) >> 16 & 0x1F)
#define GET_RD(iw) ((iw) >> 11 & 0x1F)

#define GET_FS(iw) (((iw) >> 11 & 0x1F) + VR4300_REGISTER_CP1_0)
#define GET_FT(iw) (((iw) >> 16 & 0x1F) + VR4300_REGISTER_CP1_0)
#define GET_FD(iw) (((iw) >>  6 & 0x1F) + VR4300_REGISTER_CP1_0)
#define GET_FMT(iw) ((iw) >> 21 & 0x1F)

#define OPCODE_INFO_NONE (0)
#define OPCODE_INFO_BRANCH (1 << 3)
#define OPCODE_INFO_NEEDRS (1 << 4)
#define OPCODE_INFO_NEEDFS ((1 << 4) | (1 << 1))
#define OPCODE_INFO_NEEDRT (1 << 5)
#define OPCODE_INFO_NEEDFT ((1 << 5) | (1 << 2))

enum vr4300_fmt {
  VR4300_FMT_S = 16,
  VR4300_FMT_D = 17,
  VR4300_FMT_W = 20,
  VR4300_FMT_L = 21,
};

struct vr4300_opcode {
  uint32_t id;
  uint32_t flags;
};

const struct vr4300_opcode* vr4300_decode_instruction(uint32_t);

#endif

