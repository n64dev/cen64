//
// rsp/decoder.h: RSP decoder.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __rsp_decoder_h__
#define __rsp_decoder_h__
#include "common.h"
#include "rsp/opcodes.h"

#define GET_RS(iw) ((iw) >> 21 & 0x1F)
#define GET_RT(iw) ((iw) >> 16 & 0x1F)
#define GET_RD(iw) ((iw) >> 11 & 0x1F)

#define GET_VS(iw) ((iw) >> 11 & 0x1F)
#define GET_VT(iw) ((iw) >> 16 & 0x1F)
#define GET_VD(iw) ((iw) >> 6 & 0x1F)
#define GET_E(iw) ((iw) >> 21 & 0xF)

#define OPCODE_INFO_NONE (0)
#define OPCODE_INFO_VECTOR (1 << 1)
#define OPCODE_INFO_BRANCH (1 << 2)
#define OPCODE_INFO_NEEDRS (1 << 3)
#define OPCODE_INFO_NEEDRT (1 << 4)
#define OPCODE_INFO_LOAD   (1 << 5)
#define OPCODE_INFO_STORE  (1 << 6)

struct rsp_opcode {
  uint32_t id;
  uint32_t flags;
};

const struct rsp_opcode* rsp_decode_instruction(uint32_t);

#endif

