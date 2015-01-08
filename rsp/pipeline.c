//
// rsp/pipeline.c: RSP processor pipeline.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rsp/cp0.h"
#include "rsp/cp2.h"
#include "rsp/cpu.h"
#include "rsp/decoder.h"
#include "rsp/pipeline.h"
#include "rsp/rsp.h"

// Prints out instructions and their address as they are executed.
//#define PRINT_EXEC

typedef void (*pipeline_function)(struct rsp *rsp);

// Instruction cache fetch stage.
static inline void rsp_if_stage(struct rsp *rsp) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  uint32_t pc = ifrd_latch->pc;
  uint32_t iw;

  assert(!(pc & 0x1000) || "RSP $PC points past IMEM.");

  memcpy(&iw, rsp->mem + 0x1000 + pc, sizeof(iw));
  iw = byteswap_32(iw);

  ifrd_latch->common.pc = pc;
  ifrd_latch->pc = (pc + 4) & 0xFFC;
  ifrd_latch->iw = iw;
}

// Register fetch and decode stage.
static inline int rsp_rd_stage(struct rsp *rsp) {
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;

  const struct rsp_opcode *opcode;
  uint32_t iw = ifrd_latch->iw;

  rdex_latch->common = ifrd_latch->common;

  // Check for load-use stalls.
  opcode = rsp_decode_instruction(iw);

  if (rdex_latch->opcode.flags & OPCODE_INFO_LOAD) {
    unsigned dest = GET_RT(rdex_latch->iw);
    unsigned rs = GET_RS(ifrd_latch->iw);
    unsigned rt = GET_RT(ifrd_latch->iw);

    if (unlikely((((opcode->flags & OPCODE_INFO_NEEDRS) && dest == rs)) ||
      ((opcode->flags & OPCODE_INFO_NEEDRT) && dest == rt))) {
      rdex_latch->opcode = *rsp_decode_instruction(0x00000000U);
      rdex_latch->iw = 0x00000000U;

      return 1;
    }
  }

  rdex_latch->opcode = *opcode;
  rdex_latch->iw = ifrd_latch->iw;

  return 0;
}

// Execution stage.
static inline void rsp_ex_stage(struct rsp *rsp) {
  struct rsp_dfwb_latch *dfwb_latch = &rsp->pipeline.dfwb_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;

  uint32_t rs_reg, rt_reg, temp;
  unsigned rs, rt;
  uint32_t iw;

  exdf_latch->common = rdex_latch->common;

  if (rdex_latch->opcode.flags & OPCODE_INFO_VECTOR)
    return;

  iw = rdex_latch->iw;
  rs = GET_RS(iw);
  rt = GET_RT(iw);

  // Forward results from DF/WB.
  temp = rsp->regs[dfwb_latch->result.dest];
  rsp->regs[dfwb_latch->result.dest] = dfwb_latch->result.result;
  rsp->regs[RSP_REGISTER_R0] = 0x00000000U;

  rs_reg = rsp->regs[rs];
  rt_reg = rsp->regs[rt];

  rsp->regs[dfwb_latch->result.dest] = temp;

  // Finally, execute the instruction.
#ifdef PRINT_EXEC
  debug("%.8X: %s\n", rdex_latch->common.pc,
    rsp_opcode_mnemonics[rdex_latch->opcode.id]);
#endif

  return rsp_function_table[rdex_latch->opcode.id](
    rsp, iw, rs_reg, rt_reg);
}

// Execution stage (vector).
static inline void rsp_v_ex_stage(struct rsp *rsp) {
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;

  rsp_vect_t vd_reg, vs_reg, vt_shuf_reg, zero;

  unsigned vs, vt, vd, e;
  uint32_t iw;

  if (!(rdex_latch->opcode.flags & OPCODE_INFO_VECTOR))
    return;

  iw = rdex_latch->iw;
  vs = GET_VS(iw);
  vt = GET_VT(iw);
  vd = GET_VD(iw);
  e  = GET_E (iw);

  vs_reg = rsp_vect_load_unshuffled_operand(rsp->cp2.regs[vs].e);
  vt_shuf_reg = rsp_vect_load_and_shuffle_operand(rsp->cp2.regs[vt].e, e);
  zero = rsp_vzero();

  // Finally, execute the instruction.
#ifdef PRINT_EXEC
  debug("%.8X: %s\n", rdex_latch->common.pc,
    rsp_vector_opcode_mnemonics[rdex_latch->opcode.id]);
#endif

  vd_reg = rsp_vector_function_table[rdex_latch->opcode.id](
    rsp, iw, vt_shuf_reg, vs_reg, zero);

  rsp_vect_write_operand(rsp->cp2.regs[vd].e, vd_reg);
}

// Data cache fetch stage.
static inline void rsp_df_stage(struct rsp *rsp) {
  struct rsp_dfwb_latch *dfwb_latch = &rsp->pipeline.dfwb_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  const struct rsp_mem_request *request = &exdf_latch->request;
  uint32_t addr;

  dfwb_latch->common = exdf_latch->common;
  dfwb_latch->result = exdf_latch->result;

  if (request->type == RSP_MEM_REQUEST_NONE)
    return;

  addr = request->addr & 0xFFF;

  // Vector unit DMEM access.
  if (request->type != RSP_MEM_REQUEST_INT_MEM) {
    uint16_t *regp = rsp->cp2.regs[exdf_latch->result.dest].e;
    unsigned element = request->element;
    rsp_vect_t reg, dqm;

    dqm = rsp_vect_load_unshuffled_operand(exdf_latch->request.vdqm.e);
    reg = rsp_vect_load_unshuffled_operand(regp);

    // Make sure the vector data doesn't get
    // written into the scalar part of the RF.
    dfwb_latch->result.dest = 0;
    exdf_latch->result.dest = 0;

    exdf_latch->request.vldst_func(rsp, addr, element, regp, reg, dqm);
  }

  // Scalar unit DMEM access.
  else {
    uint32_t rdqm = request->rdqm;
    uint32_t wdqm = request->wdqm;
    uint32_t data = request->data;
    unsigned rshift = request->rshift;
    uint32_t word;

    memcpy(&word, rsp->mem + addr, sizeof(word));

    word = byteswap_32(word);
    dfwb_latch->result.result = rdqm & (((int32_t) word) >> rshift);
    word = byteswap_32((word & ~wdqm) | (data & wdqm));

    memcpy(rsp->mem + addr, &word, sizeof(word));
  }
}

// Writeback stage.
static inline void rsp_wb_stage(struct rsp *rsp) {
  const struct rsp_dfwb_latch *dfwb_latch = &rsp->pipeline.dfwb_latch;

  rsp->regs[dfwb_latch->result.dest] = dfwb_latch->result.result;
}

// Advances the processor pipeline by one clock.
void rsp_cycle(struct rsp *rsp) {
  if (rsp->regs[RSP_CP0_REGISTER_SP_STATUS] & SP_STATUS_HALT)
    return;

  rsp_wb_stage(rsp);
  rsp_df_stage(rsp);

  rsp->pipeline.exdf_latch.result.dest = RSP_REGISTER_R0;
  rsp->pipeline.exdf_latch.request.type = RSP_MEM_REQUEST_NONE;

  rsp_v_ex_stage(rsp);
  rsp_ex_stage(rsp);

  if (!rsp_rd_stage(rsp))
    rsp_if_stage(rsp);
}

// Initializes the pipeline with default values.
void rsp_pipeline_init(struct rsp_pipeline *pipeline) {
  memset(pipeline, 0, sizeof(*pipeline));
}

