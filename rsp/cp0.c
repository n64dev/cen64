//
// rsp/cp0.c: RSP control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/controller.h"
#include "rsp/cp0.h"
#include "rsp/cpu.h"
#include "rsp/decoder.h"
#include "rsp/interface.h"
#include "vr4300/interface.h"

static void rsp_status_write(struct rsp *rsp, uint32_t rt);

//
// MFC0
//
void RSP_MFC0(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdc_latch *exdc_latch = &rsp->pipeline.exdc_latch;
  unsigned dest;

  dest = GET_RT(iw);
  rt = rsp_read_cp0_reg(rsp, GET_RD(iw));

  exdc_latch->result = rt;
  exdc_latch->dest = dest;
}

//
// MTC0
//
void RSP_MTC0(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  unsigned dest;

  dest = GET_RD(iw);
  rsp_write_cp0_reg(rsp, dest, rt);
}

// Reads a value from the control coprocessor.
uint32_t rsp_read_cp0_reg(struct rsp *rsp, unsigned src) {
  src = SP_REGISTER_OFFSET + src;

  switch(src) {
    case RSP_CP0_REGISTER_SP_RESERVED:
      if (!rsp->regs[RSP_CP0_REGISTER_SP_RESERVED]) {
        rsp->regs[RSP_CP0_REGISTER_SP_RESERVED] = 1;
        return 0;
      }

      return 1;

    case RSP_CP0_REGISTER_DMA_FULL:
    case RSP_CP0_REGISTER_DMA_BUSY:
      return 0;

    // RDP aliases.
    case RSP_CP0_REGISTER_CMD_START:
    case RSP_CP0_REGISTER_CMD_END:
    case RSP_CP0_REGISTER_CMD_CURRENT:
    case RSP_CP0_REGISTER_CMD_STATUS:
    case RSP_CP0_REGISTER_CMD_CLOCK:
    case RSP_CP0_REGISTER_CMD_BUSY:
    case RSP_CP0_REGISTER_CMD_PIPE_BUSY:
    case RSP_CP0_REGISTER_CMD_TMEM_BUSY:
      abort();

    default:
      return rsp->regs[src];
  }

  return 0;
}

// Updates the SP_STATUS register according to bitmask in rt.
void rsp_status_write(struct rsp *rsp, uint32_t rt) {
  uint32_t status = rsp->regs[RSP_CP0_REGISTER_CMD_STATUS];

  if (rt & SP_CLR_HALT)
    status &= ~SP_STATUS_HALT;
  else if (rt & SP_SET_HALT)
    status |= SP_STATUS_HALT;

  if (rt & SP_CLR_BROKE)
    status &= ~SP_STATUS_BROKE;

  if (rt & SP_CLR_INTR)
    signal_rcp_interrupt(rsp->bus->vr4300, MI_INTR_SP);
  else if (rt & SP_SET_INTR)
    clear_rcp_interrupt(rsp->bus->vr4300, MI_INTR_SP);

  if (rt & SP_CLR_SSTEP)
    status &= ~SP_STATUS_SSTEP;
  else if (rt & SP_SET_SSTEP)
    status |= SP_STATUS_SSTEP;

  if (rt & SP_CLR_INTR_BREAK)
    status &= ~SP_STATUS_INTR_BREAK;
  else if  (rt & SP_SET_INTR_BREAK)
    status |= SP_STATUS_INTR_BREAK;

  if (rt & SP_CLR_SIG0)
    status &= ~SP_STATUS_SIG0;
  else if (rt & SP_SET_SIG0)
    status |= SP_STATUS_SIG0;

  if (rt & SP_CLR_SIG1)
    status &= ~SP_STATUS_SIG1;
  else if (rt & SP_SET_SIG1)
    status |= SP_STATUS_SIG1;

  if (rt & SP_CLR_SIG2)
    status &= ~SP_STATUS_SIG2;
  else if (rt & SP_SET_SIG2)
    status |= SP_STATUS_SIG2;

  if (rt & SP_CLR_SIG3)
    status &= ~SP_STATUS_SIG3;
  else if (rt & SP_SET_SIG3)
    status |= SP_STATUS_SIG3;

  if (rt & SP_CLR_SIG4)
    status &= ~SP_STATUS_SIG4;
  else if (rt & SP_SET_SIG4)
    status |= SP_STATUS_SIG4;

  if (rt & SP_CLR_SIG5)
    status &= ~SP_STATUS_SIG5;
  else if (rt & SP_SET_SIG5)
    status |= SP_STATUS_SIG5;

  if (rt & SP_CLR_SIG6)
    status &= ~SP_STATUS_SIG6;
  else if (rt & SP_SET_SIG6)
    status |= SP_STATUS_SIG6;

  if (rt & SP_CLR_SIG7)
    status &= ~SP_STATUS_SIG7;
  else if (rt & SP_SET_SIG7)
    status |= SP_STATUS_SIG7;

  rsp->regs[RSP_CP0_REGISTER_CMD_STATUS] = status;
}

// Writes a value to the control processor.
void rsp_write_cp0_reg(struct rsp *rsp, unsigned dest, uint32_t rt) {
  dest = SP_REGISTER_OFFSET + dest;

  switch(dest) {
    case RSP_CP0_REGISTER_DMA_CACHE:
      rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] = rt & 0x1FFF;
      break;

    case RSP_CP0_REGISTER_DMA_DRAM:
      rsp->regs[RSP_CP0_REGISTER_DMA_DRAM] = rt & 0xFFFFFF;
      break;

    case RSP_CP0_REGISTER_DMA_READ_LENGTH:
      rsp->regs[RSP_CP0_REGISTER_DMA_READ_LENGTH] = rt;
      //rsp_dma_read(rsp);
      break;

    case RSP_CP0_REGISTER_DMA_WRITE_LENGTH:
      rsp->regs[RSP_CP0_REGISTER_DMA_WRITE_LENGTH] = rt;
      //rsp_dma_write(rsp);
      break;

    case RSP_CP0_REGISTER_SP_STATUS:
      rsp_status_write(rsp, rt);
      break;

    case RSP_CP0_REGISTER_SP_RESERVED:
      if (rt == 0)
        rsp->regs[RSP_CP0_REGISTER_SP_RESERVED] = 0;

      break;

    // RDP aliases.
    case RSP_CP0_REGISTER_CMD_START:
    case RSP_CP0_REGISTER_CMD_END:
    case RSP_CP0_REGISTER_CMD_CURRENT:
    case RSP_CP0_REGISTER_CMD_STATUS:
    case RSP_CP0_REGISTER_CMD_CLOCK:
    case RSP_CP0_REGISTER_CMD_BUSY:
    case RSP_CP0_REGISTER_CMD_PIPE_BUSY:
    case RSP_CP0_REGISTER_CMD_TMEM_BUSY:
      abort();

    default:
      rsp->regs[dest] = rt;
      break;
  }
}

// Initializes the coprocessor.
void rsp_cp0_init(struct rsp *rsp) {
  rsp->regs[RSP_CP0_REGISTER_SP_STATUS] = 0x1;
}

