//
// rsp/functions.c: RSP execution functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#define RSP_BUILD_OP(op, func, flags) \
  (RSP_##func)

#include "common.h"
#include "bus/controller.h"
#include "rsp/cp0.h"
#include "rsp/cpu.h"
#include "rsp/decoder.h"
#include "rsp/opcodes.h"
#include "rsp/opcodes_priv.h"
#include "rsp/pipeline.h"
#include "vr4300/interface.h"

// Mask to negate second operand if subtract operation.
cen64_align(static const uint32_t rsp_addsub_lut[4], 16) = {
  0x0U, ~0x0U, ~0x0U, ~0x0U
};

// Mask to select outputs for bitwise operations.
cen64_align(static const uint32_t rsp_bitwise_lut[4][2], 32) = {
  {~0U,  0U}, // AND
  {~0U, ~0U}, // OR
  { 0U, ~0U}, // XOR
  { 0U,  0U}, // -
};

// Mask to denote which part of the vector to load/store.
cen64_align(static const uint16_t rsp_bdls_lut[4][4], CACHE_LINE_SIZE) = {
  {0xFF00, 0x0000, 0x0000, 0x0000}, // B
  {0xFFFF, 0x0000, 0x0000, 0x0000}, // S
  {0xFFFF, 0xFFFF, 0x0000, 0x0000}, // L
  {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF}, // D
};

cen64_align(static const uint16_t rsp_qr_lut[16][8], CACHE_LINE_SIZE) = {
  {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x00FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0x00FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},

  {0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0x0000, 0x00FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0x00FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},

  {0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x00FF, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00FF, 0xFFFF, 0xFFFF},

  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00FF, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00FF},
};

// Mask to select link address register for some branches.
cen64_align(static const uint32_t rsp_branch_lut[2], 8) = {
  ~0U, 0U
};

// Mask to selectively sign-extend loaded values.
cen64_align(static const uint32_t rsp_load_sex_mask[2][4], 32) = {
  {~0U,   ~0U,     0U, ~0U}, // sex
  {0xFFU, 0xFFFFU, 0U, ~0U}, // zex
};

// Function to sign-extend 6-bit values.
static inline unsigned sign_extend_6(int i) {
  return i | -(i & 0x40);
}

//
// ADDIU
// LUI
// SUBIU
//
void RSP_ADDIU_LUI_SUBIU(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned immshift = iw >> 24 & 0x10;
  unsigned dest;

  dest = GET_RT(iw);

  rt = (int16_t) iw;
  rt = rs + (rt << immshift);

  exdf_latch->result.result = rt;
  exdf_latch->result.dest = dest;
}

//
// ADDU
// SUBU
//
void RSP_ADDU_SUBU(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  uint32_t mask = rsp_addsub_lut[iw & 0x2];

  unsigned dest;
  uint32_t rd;

  dest = GET_RD(iw);
  rt = (rt ^ mask) - mask;
  rd = rs + rt;

  exdf_latch->result.result = rd;
  exdf_latch->result.dest = dest;
}

//
// AND
// OR
// XOR
//
void RSP_AND_OR_XOR(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  uint32_t and_mask = rsp_bitwise_lut[iw & 0x3][0];
  uint32_t xor_mask = rsp_bitwise_lut[iw & 0x3][1];

  unsigned dest;
  uint32_t rd;

  dest = GET_RD(iw);
  rd = ((rs & rt) & and_mask) | ((rs ^ rt) & xor_mask);

  exdf_latch->result.result = rd;
  exdf_latch->result.dest = dest;
}

//
// ANDI
// ORI
// XORI
//
void RSP_ANDI_ORI_XORI(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  uint32_t and_mask = rsp_bitwise_lut[iw >> 26 & 0x3][0];
  uint32_t xor_mask = rsp_bitwise_lut[iw >> 26 & 0x3][1];

  unsigned dest;

  dest = GET_RT(iw);
  rt = (uint16_t) iw;
  rt = ((rs & rt) & and_mask) | ((rs ^ rt) & xor_mask);

  exdf_latch->result.result = rt;
  exdf_latch->result.dest = dest;
}

//
// BEQ
// BNE
//
// TODO: Adjust branch target for RSP IMEM range.
//
void RSP_BEQ_BNE(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  uint32_t offset = (uint32_t) ((int16_t) iw) << 2;

  bool is_ne = iw >> 26 & 0x1;
  bool cmp = rs == rt;

  if (cmp == is_ne)
    return;

  ifrd_latch->pc = (rdex_latch->common.pc + offset + 4) & 0xFFC;
}

//
// BGEZ
// BLTZ
//
// TODO: Adjust branch target for RSP IMEM range.
//
void RSP_BGEZ_BLTZ(
  struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t unused(rt)) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  uint32_t offset = (uint32_t) ((int16_t) iw) << 2;

  bool is_ge = iw >> 16 & 0x1;
  bool cmp = (int32_t) rs < 0;

  if (cmp == is_ge)
    return;

  ifrd_latch->pc = (rdex_latch->common.pc + offset + 4) & 0xFFC;
}

//
// BGEZAL
// BLTZAL
//
// TODO: Adjust branch target for RSP IMEM range.
//
void RSP_BGEZAL_BLTZAL(
  struct rsp *rsp, uint32_t iw, uint32_t rs, uint32_t unused(rt)) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  uint32_t offset = (uint32_t) ((int16_t) iw) << 2;

  bool is_ge = iw >> 16 & 0x1;
  bool cmp = (int32_t) rs < 0;

  exdf_latch->result.result = 0x1000 | (rdex_latch->common.pc + 8);
  exdf_latch->result.dest = RSP_REGISTER_RA;

  if (cmp == is_ge)
    return;

  ifrd_latch->pc = (rdex_latch->common.pc + offset + 4) & 0xFFC;
}

//
// BGTZ
// BLEZ
//
// TODO: Adjust branch target for RSP IMEM range.
//
void RSP_BGTZ_BLEZ(
  struct rsp *rsp, uint32_t iw, uint32_t rs, uint32_t unused(rt)) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  uint32_t offset = (uint32_t) ((int16_t) iw) << 2;

  bool is_gt = iw >> 26 & 0x1;
  bool cmp = (int32_t) rs <= 0;

  if (cmp == is_gt)
    return;

  ifrd_latch->pc = (rdex_latch->common.pc + offset + 4) & 0xFFC;
}

//
// BREAK
//
void RSP_BREAK(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  uint32_t result = rsp->regs[RSP_CP0_REGISTER_SP_STATUS] |
    (SP_STATUS_HALT | SP_STATUS_BROKE);

  if (rsp->regs[RSP_CP0_REGISTER_SP_STATUS] & SP_STATUS_INTR_BREAK)
    signal_rcp_interrupt(rsp->bus->vr4300, MI_INTR_SP);

  exdf_latch->result.dest = RSP_CP0_REGISTER_SP_STATUS;
  exdf_latch->result.result = result;
}

//
// LB
// LBU
// LH
// LHU
// LW
// SB
// SH
// SW
//
cen64_hot void RSP_INT_MEM(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  uint32_t address = rs + (int16_t) iw;
  uint32_t sel_mask = (int32_t) (iw << 2) >> 31;
  unsigned request_size = (iw >> 26 & 0x3);
  unsigned lshiftamt = (3 - request_size) << 3;
  uint32_t rdqm = rsp_load_sex_mask[iw >> 28 & 0x1][request_size];
  uint32_t wdqm = ~0U << lshiftamt;

  exdf_latch->request.addr = address;
  exdf_latch->request.data = rt << lshiftamt;
  exdf_latch->request.rdqm = rdqm;
  exdf_latch->request.type = RSP_MEM_REQUEST_INT_MEM;
  exdf_latch->request.rshift = lshiftamt;
  exdf_latch->request.wdqm = sel_mask & wdqm;

  exdf_latch->result.dest = ~sel_mask & GET_RT(iw);
}

//
// INVALID
//
void RSP_INVALID(struct rsp *rsp,
  uint32_t iw, uint32_t unused(rs), uint32_t unused(rt)) {
#ifndef NDEBUG
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;

  debug("Unimplemented instruction: %s [0x%.8X] @ 0x%.8X\n",
    rsp_opcode_mnemonics[rdex_latch->opcode.id],
    iw, rdex_latch->common.pc);
#endif
}

//
// J
// JAL
//
void RSP_J_JAL(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  bool is_jal = iw >> 26 & 0x1;
  uint32_t target = iw << 2 & 0xFFC;
  uint32_t mask = rsp_branch_lut[is_jal];

  exdf_latch->result.result = 0x1000 | ((rdex_latch->common.pc + 8) & 0xFFC);
  exdf_latch->result.dest = RSP_REGISTER_RA & ~mask;

  ifrd_latch->pc = target;
}

//
// JALR
// JR
//
void RSP_JALR_JR(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t unused(rt)) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  bool is_jalr = iw & 0x1;
  uint32_t mask = rsp_branch_lut[is_jalr];
  unsigned rd = GET_RD(iw);

  exdf_latch->result.result = 0x1000 | ((rdex_latch->common.pc + 8) & 0xFFC);
  exdf_latch->result.dest = rd & ~mask;

  ifrd_latch->pc = rs & 0xFFC;
}

//
// LBV
// LDV
// LLV
// LSV
// SBV
// SDV
// SLV
// SSV
//
void RSP_LBDLSV_SBDLSV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned shift_and_idx = iw >> 11 & 0x3;
  unsigned dest = GET_VT(iw);

  exdf_latch->request.addr = rs + (sign_extend_6(iw & 0x7F) << shift_and_idx);

  __m128i vdqm = _mm_loadl_epi64((__m128i *) (rsp_bdls_lut[shift_and_idx]));
  _mm_store_si128((__m128i *) exdf_latch->request.vdqm.e, vdqm);

  exdf_latch->request.element = GET_EL(iw);
  exdf_latch->request.type = RSP_MEM_REQUEST_VECTOR;
  exdf_latch->request.vldst_func = (iw >> 29 & 0x1)
    ? rsp_vstore_group1
    : rsp_vload_group1;

  exdf_latch->result.dest = dest;
}

//
// LPV
// LUV
// SPV
// SUV
//
void RSP_LFHPUV_SFHPUV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_VT(iw);

  static const enum rsp_mem_request_type fhpu_type_lut[4] = {
    RSP_MEM_REQUEST_PACK,
    RSP_MEM_REQUEST_UPACK,
    RSP_MEM_REQUEST_HALF,
    RSP_MEM_REQUEST_FOURTH
  };

  exdf_latch->request.addr = rs + (sign_extend_6(iw & 0x7F) << 3);

//  memcpy(&exdf_latch->request.vdqm.e,
//    rsp_fhpu_lut[exdf_latch->request.addr & 0xF],
//    sizeof(exdf_latch->request.vdqm.e));

  exdf_latch->request.type = fhpu_type_lut[(iw >> 11 & 0x1F) - 6];
  exdf_latch->request.vldst_func = (iw >> 29 & 0x1)
    ? rsp_vstore_group2
    : rsp_vload_group2;

  exdf_latch->result.dest = dest;
}

//
// LQV
// LRV
// SQV
// SRV
//
void RSP_LQRV_SQRV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_VT(iw);

  exdf_latch->request.addr = rs + (sign_extend_6(iw & 0x7F) << 4);

  memcpy(exdf_latch->request.vdqm.e,
    rsp_qr_lut[exdf_latch->request.addr & 0xF],
    sizeof(exdf_latch->request.vdqm.e));

  exdf_latch->request.element = GET_EL(iw);
  exdf_latch->request.type = (iw >> 11 & 0x1)
    ? RSP_MEM_REQUEST_REST
    : RSP_MEM_REQUEST_QUAD;

  exdf_latch->request.vldst_func = (iw >> 29 & 0x1)
    ? rsp_vstore_group4
    : rsp_vload_group4;

  exdf_latch->result.dest = dest;
}

//
// NOR
//
void RSP_NOR(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);

  exdf_latch->result.result = ~(rs | rt);
  exdf_latch->result.dest = dest;
}

//
// SLL
// SLLV
//
void RSP_SLL_SLLV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = (rs & 0x1F) + (iw >> 6 & 0x1F);

  exdf_latch->result.result = rt << sa;
  exdf_latch->result.dest = dest;
}

//
// SLT
//
void RSP_SLT(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);

  exdf_latch->result.result = (int32_t) rs < (int32_t) rt;
  exdf_latch->result.dest = dest;
}

//
// SLTI
//
void RSP_SLTI(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  unsigned dest = GET_RT(iw);
  int32_t imm = (int16_t) iw;

  exdf_latch->result.result = (int32_t) rs < imm;
  exdf_latch->result.dest = dest;
}

//
// SLTIU
//
void RSP_SLTIU(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  unsigned dest = GET_RT(iw);
  uint32_t imm = (int16_t) iw;

  exdf_latch->result.result = rs < imm;
  exdf_latch->result.dest = dest;
}

//
// SLTU
//
void RSP_SLTU(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);

  exdf_latch->result.result = rs < rt;
  exdf_latch->result.dest = dest;
}

//
// SRA
//
void RSP_SRA(struct rsp *rsp,
  uint32_t iw, uint32_t unused(rs), uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = iw >> 6 & 0x1F;

  exdf_latch->result.result = (int32_t) rt >> sa;
  exdf_latch->result.dest = dest;
}

//
// SRAV
//
void RSP_SRAV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x1F;

  exdf_latch->result.result = (int32_t) rt >> sa;
  exdf_latch->result.dest = dest;
}

//
// SRL
//
void RSP_SRL(struct rsp *rsp,
  uint32_t iw, uint32_t unused(rs), uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = iw >> 6 & 0x1F;

  exdf_latch->result.result = rt >> sa;
  exdf_latch->result.dest = dest;
}

//
// SRLV
//
void RSP_SRLV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x1F;

  exdf_latch->result.result = rt >> sa;
  exdf_latch->result.dest = dest;
}

// Function lookup table.
cen64_align(const rsp_function
  rsp_function_table[NUM_RSP_OPCODES], CACHE_LINE_SIZE) = {
#define X(op) op,
#include "rsp/opcodes.md"
#undef X
};

