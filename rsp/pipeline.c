//
// rsp/pipeline.c: RSP processor pipeline.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rsp/cp0.h"
#include "rsp/cp2.h"
#include "rsp/cpu.h"
#include "rsp/decoder.h"
#include "rsp/interface.h"
#include "rsp/opcodes.h"
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
  ifrd_latch->pc = (pc + 4) & 0xFFC;

  memcpy(&iw, rsp->mem + 0x1000 + pc, sizeof(iw));

  ifrd_latch->common.pc = pc;
  ifrd_latch->opcode = rsp->opcode_cache[pc >> 2];
  ifrd_latch->iw = iw;
}

// Register fetch and decode stage.
static inline int rsp_rd_stage(struct rsp *rsp) {
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;

  uint32_t previous_insn_flags = rdex_latch->opcode.flags;
  uint32_t iw = ifrd_latch->iw;

  rdex_latch->common = ifrd_latch->common;
  rdex_latch->opcode = ifrd_latch->opcode;
  rdex_latch->iw = iw;

  // Check for load-use stalls.
  if (previous_insn_flags & OPCODE_INFO_LOAD) {
    const struct rsp_opcode *opcode = &rdex_latch->opcode;
    unsigned dest = rsp->pipeline.exdf_latch.result.dest;
    unsigned rs = GET_RS(iw);
    unsigned rt = GET_RT(iw);

    if (unlikely(dest && (
      (dest == rs && (opcode->flags & OPCODE_INFO_NEEDRS)) ||
      (dest == rt && (opcode->flags & OPCODE_INFO_NEEDRT))
    ))) {
      static const struct rsp_opcode rsp_rf_kill_op = {RSP_OPCODE_SLL, 0x0};

      rdex_latch->opcode = rsp_rf_kill_op;
      rdex_latch->iw = 0x00000000U;

      return 1;
    }
  }

  return 0;
}

// Execution stage.
cen64_flatten static inline void rsp_ex_stage(struct rsp *rsp) {
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
cen64_flatten static inline void rsp_v_ex_stage(struct rsp *rsp) {
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
cen64_flatten static inline void rsp_df_stage(struct rsp *rsp) {
  struct rsp_dfwb_latch *dfwb_latch = &rsp->pipeline.dfwb_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  const struct rsp_mem_request *request = &exdf_latch->request;
  uint32_t addr;

  dfwb_latch->common = exdf_latch->common;
  dfwb_latch->result = exdf_latch->result;

  if (request->type == RSP_MEM_REQUEST_NONE)
    return;

  addr = request->addr & 0xFFF;

  // Scalar unit DMEM access.
  if (request->type == RSP_MEM_REQUEST_INT_MEM) {
    uint32_t rdqm = request->packet.p_int.rdqm;
    uint32_t wdqm = request->packet.p_int.wdqm;
    uint32_t data = request->packet.p_int.data;
    unsigned rshift = request->packet.p_int.rshift;
    uint32_t word;

    memcpy(&word, rsp->mem + addr, sizeof(word));

    word = byteswap_32(word);
    dfwb_latch->result.result = rdqm & (((int32_t) word) >> rshift);
    word = byteswap_32((word & ~wdqm) | (data & wdqm));

    memcpy(rsp->mem + addr, &word, sizeof(word));
  }
  // Transposed vector unit DMEM access.
  else if (request->type == RSP_MEM_REQUEST_TRANSPOSE) {
    unsigned element = request->packet.p_transpose.element;
    unsigned vt = request->packet.p_transpose.vt;

    exdf_latch->request.packet.p_transpose.transpose_func(
      rsp, addr, element, vt);
  }
  // Vector unit DMEM access.
  else {
    uint16_t *regp = rsp->cp2.regs[request->packet.p_vect.dest].e;
    unsigned element = request->packet.p_vect.element;
    rsp_vect_t reg, dqm;

    reg = rsp_vect_load_unshuffled_operand(regp);
    dqm = rsp_vect_load_unshuffled_operand(exdf_latch->
      request.packet.p_vect.vdqm.e);

    // Make sure the vector data doesn't get
    // written into the scalar part of the RF.
    dfwb_latch->result.dest = 0;

    exdf_latch->request.packet.p_vect.vldst_func(
      rsp, addr, element, regp, reg, dqm);
  }

}

// Writeback stage.
static inline void rsp_wb_stage(struct rsp *rsp) {
  const struct rsp_dfwb_latch *dfwb_latch = &rsp->pipeline.dfwb_latch;

  rsp->regs[dfwb_latch->result.dest] = dfwb_latch->result.result;
}

// Advances the processor pipeline by one clock.
void rsp_cycle_(struct rsp *rsp) {
  rsp_wb_stage(rsp);

  rsp_df_stage(rsp);

  rsp->pipeline.exdf_latch.result.dest = RSP_REGISTER_R0;
  rsp->pipeline.exdf_latch.request.type = RSP_MEM_REQUEST_NONE;

  debug_check_breakpoints(&rsp->debug, rsp_get_pc(rsp));

  rsp_v_ex_stage(rsp);
  rsp_ex_stage(rsp);

  if (likely(!rsp_rd_stage(rsp)))
    rsp_if_stage(rsp);
}

// Initializes the pipeline with default values.
void rsp_pipeline_init(struct rsp_pipeline *pipeline) {
  memset(pipeline, 0, sizeof(*pipeline));
}

