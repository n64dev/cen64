//
// rsp/cp0.c: RSP control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __rsp_cp0_h__
#define __rsp_cp0_h__
#include "common.h"

// SP_STATUS read bits.
#define SP_STATUS_HALT            0x0001
#define SP_STATUS_BROKE           0x0002
#define SP_STATUS_DMA_BUSY        0x0004
#define SP_STATUS_DMA_FULL        0x0008
#define SP_STATUS_IO_FULL         0x0010
#define SP_STATUS_SSTEP           0x0020
#define SP_STATUS_INTR_BREAK      0x0040
#define SP_STATUS_SIG0            0x0080
#define SP_STATUS_SIG1            0x0100
#define SP_STATUS_SIG2            0x0200
#define SP_STATUS_SIG3            0x0400
#define SP_STATUS_SIG4            0x0800
#define SP_STATUS_SIG5            0x1000
#define SP_STATUS_SIG6            0x2000
#define SP_STATUS_SIG7            0x4000

// SP_STATUS write bits.
#define SP_CLR_HALT               0x00000001
#define SP_SET_HALT               0x00000002
#define SP_CLR_BROKE              0x00000004
#define SP_CLR_INTR               0x00000008
#define SP_SET_INTR               0x00000010
#define SP_CLR_SSTEP              0x00000020
#define SP_SET_SSTEP              0x00000040
#define SP_CLR_INTR_BREAK         0x00000080
#define SP_SET_INTR_BREAK         0x00000100
#define SP_CLR_SIG0               0x00000200
#define SP_SET_SIG0               0x00000400
#define SP_CLR_SIG1               0x00000800
#define SP_SET_SIG1               0x00001000
#define SP_CLR_SIG2               0x00002000
#define SP_SET_SIG2               0x00004000
#define SP_CLR_SIG3               0x00008000
#define SP_SET_SIG3               0x00010000
#define SP_CLR_SIG4               0x00020000
#define SP_SET_SIG4               0x00040000
#define SP_CLR_SIG5               0x00080000
#define SP_SET_SIG5               0x00100000
#define SP_CLR_SIG6               0x00200000
#define SP_SET_SIG6               0x00400000
#define SP_CLR_SIG7               0x00800000
#define SP_SET_SIG7               0x01000000

struct rsp;

// Registers list.
enum rsp_cp0_register {
  RSP_CP0_REGISTER_DMA_CACHE = 32,
  RSP_CP0_REGISTER_DMA_DRAM = 33,
  RSP_CP0_REGISTER_DMA_READ_LENGTH = 34,
  RSP_CP0_REGISTER_DMA_WRITE_LENGTH = 35,
  RSP_CP0_REGISTER_SP_STATUS = 36,
  RSP_CP0_REGISTER_DMA_FULL = 37,
  RSP_CP0_REGISTER_DMA_BUSY = 38,
  RSP_CP0_REGISTER_SP_RESERVED = 39,

  RSP_CP0_REGISTER_CMD_START = 40,
  RSP_CP0_REGISTER_CMD_END = 41,
  RSP_CP0_REGISTER_CMD_CURRENT = 42,
  RSP_CP0_REGISTER_CMD_STATUS = 43,
  RSP_CP0_REGISTER_CMD_CLOCK = 44,
  RSP_CP0_REGISTER_CMD_BUSY = 45,
  RSP_CP0_REGISTER_CMD_PIPE_BUSY = 46,
  RSP_CP0_REGISTER_CMD_TMEM_BUSY = 47,
};

void RSP_MFC0(struct rsp *rsp, uint32_t iw, uint32_t rs, uint32_t rt);
void RSP_MTC0(struct rsp *rsp, uint32_t iw, uint32_t rs, uint32_t rt);

uint32_t rsp_read_cp0_reg(struct rsp *rsp, unsigned src);
void rsp_write_cp0_reg(struct rsp *rsp, unsigned dest, uint32_t rt);

cen64_cold void rsp_cp0_init(struct rsp *rsp);

#endif

