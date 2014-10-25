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
cen64_align(static const uint16_t rsp_bdlqs_lut[5][8], CACHE_LINE_SIZE) = {
  {0xFF00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, // B
  {0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, // S
  {0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, // L
  {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000}, // D
  {0xFFFF, 0xFFFF, 0xFFFF, 0XFFFF, 0xFFFF, 0xFFFF, 0XFFFF, 0XFFFF}, // Q
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

  exdf_latch->result = rt;
  exdf_latch->dest = dest;
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

  exdf_latch->result = rd;
  exdf_latch->dest = dest;
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

  exdf_latch->result = rd;
  exdf_latch->dest = dest;
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

  exdf_latch->result = rt;
  exdf_latch->dest = dest;
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

  ifrd_latch->pc = rdex_latch->common.pc + (offset + 4);
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

  ifrd_latch->pc = rdex_latch->common.pc + (offset + 4);
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

  exdf_latch->result = rdex_latch->common.pc + 8;
  exdf_latch->dest = RSP_REGISTER_RA;

  if (cmp == is_ge)
    return;

  ifrd_latch->pc = rdex_latch->common.pc + (offset + 4);
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

  ifrd_latch->pc = rdex_latch->common.pc + (offset + 4);
}

//
// BREAK
//
void RSP_BREAK(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  rsp->regs[RSP_CP0_REGISTER_SP_STATUS] |= (SP_STATUS_HALT | SP_STATUS_BROKE);

  if (rsp->regs[RSP_CP0_REGISTER_SP_STATUS] & SP_STATUS_INTR_BREAK)
    signal_rcp_interrupt(rsp->bus->vr4300, MI_INTR_SP);

  // Kill whatever's behind us; not sure if this is necessary?
  rdex_latch->opcode = *rsp_decode_instruction(0x00000000U);
  rdex_latch->iw = 0x00000000U;
}

//
// INV
//
void RSP_INV(struct rsp *rsp,
  uint32_t iw, uint32_t unused(rs), uint32_t unused(rt)) {
#ifndef NDEBUG
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;

  debug("Unimplemented instruction: %s [0x%.8X] @ 0x%.16llX\n",
    rsp_opcode_mnemonics[rdex_latch->opcode.id], iw, (long long unsigned)
    rdex_latch->common.pc);
#endif
}

//
// J
// JAL
//
// TODO: Adjust branch target for RSP IMEM range.
//
void RSP_J_JAL(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  bool is_jal = iw >> 26 & 0x1;
  uint32_t target = iw << 2 & 0x3FF;
  uint32_t mask = rsp_branch_lut[is_jal];

  exdf_latch->result = rdex_latch->common.pc + 8;
  exdf_latch->dest = RSP_REGISTER_RA & ~mask;

  ifrd_latch->pc = target;
}

//
// JALR
// JR
//
// TODO: Adjust branch target for RSP IMEM range.
//
void RSP_JALR_JR(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t unused(rt)) {
  struct rsp_ifrd_latch *ifrd_latch = &rsp->pipeline.ifrd_latch;
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  bool is_jalr = iw & 0x1;
  uint32_t mask = rsp_branch_lut[is_jalr];

  exdf_latch->result = rdex_latch->common.pc + 8;
  exdf_latch->dest = RSP_REGISTER_RA & ~mask;

  ifrd_latch->pc = rs;
}

//
// LB
// LBU
// LH
// LHU
// LW
//
// TODO/FIXME: Check for unaligned addresses.
//
void RSP_LOAD(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t unused(rt)) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  unsigned request_size = (iw >> 26 & 0x3);
  uint32_t dqm = rsp_load_sex_mask[iw >> 28 & 0x1][request_size];
  unsigned dest = GET_RT(iw);

  exdf_latch->request.addr = rs + (int16_t) iw;
  exdf_latch->request.dqm = dqm;
  exdf_latch->request.type = RSP_MEM_REQUEST_READ;
  exdf_latch->request.size = request_size + 1;

  exdf_latch->dest = dest;
}

//
// LUI
//
void RSP_LUI(struct rsp *rsp,
  uint32_t iw, uint32_t unused(rs), uint32_t unused(rt)) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  int32_t imm = iw << 16;
  unsigned dest = GET_RT(iw);

  exdf_latch->result = imm;
  exdf_latch->dest = dest;
}

//
// LBV
// LDV
// LLV
// LQV
// LSV
// SBV
// SDV
// SLV
// SQV
// SSV
//
void RSP_BDLQSV_SBDLQSV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned shift_and_idx = iw >> 11 & 0x7;
  unsigned dest = GET_VT(iw);

  exdf_latch->request.addr = rs + ((uint16_t) iw << shift_and_idx);
  memcpy(exdf_latch->request.vdqm, rsp_bdlqs_lut[shift_and_idx],
    sizeof(exdf_latch->request.vdqm));

  exdf_latch->request.element = GET_E(iw);
  exdf_latch->request.type = (iw >> 29 & 0x1)
    ? RSP_MEM_REQUEST_VECTOR_WRITE
    : RSP_MEM_REQUEST_VECTOR_READ;

  exdf_latch->dest = dest + 32;
}

//
// NOR
//
void RSP_NOR(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);

  exdf_latch->result = ~(rs | rt);
  exdf_latch->dest = dest;
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

  exdf_latch->result = rt << sa;
  exdf_latch->dest = dest;
}

//
// SLT
//
void RSP_SLT(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);

  exdf_latch->result = (int32_t) rs < (int32_t) rt;
  exdf_latch->dest = dest;
}

//
// SLTI
//
void RSP_SLTI(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  unsigned dest = GET_RT(iw);
  int32_t imm = (int16_t) iw;

  exdf_latch->result = (int32_t) rs < imm;
  exdf_latch->dest = dest;
}

//
// SLTIU
//
void RSP_SLTIU(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  unsigned dest = GET_RT(iw);
  uint32_t imm = (int16_t) iw;

  exdf_latch->result = rs < imm;
  exdf_latch->dest = dest;
}

//
// SLTU
//
void RSP_SLTU(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);

  exdf_latch->result = rs < rt;
  exdf_latch->dest = dest;
}

//
// SRA
//
void RSP_SRA(struct rsp *rsp,
  uint32_t iw, uint32_t unused(rs), uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = iw >> 6 & 0x1F;

  exdf_latch->result = (int32_t) rt >> sa;
  exdf_latch->dest = dest;
}

//
// SRAV
//
void RSP_SRAV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x1F;

  exdf_latch->result = (int32_t) rt >> sa;
  exdf_latch->dest = dest;
}

//
// SRL
//
void RSP_SRL(struct rsp *rsp,
  uint32_t iw, uint32_t unused(rs), uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = iw >> 6 & 0x1F;

  exdf_latch->result = rt >> sa;
  exdf_latch->dest = dest;
}

//
// SRLV
//
void RSP_SRLV(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x1F;

  exdf_latch->result = rt >> sa;
  exdf_latch->dest = dest;
}

//
// SB
// SH
// SW
//
// TODO/FIXME: Check for unaligned addresses.
//
void RSP_STORE(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  uint32_t address = rs + (int16_t) iw;
  unsigned request_size = (iw >> 26 & 0x3) + 1;
  unsigned lshiftamt = (4 - request_size) << 3;
  unsigned rshiftamt = (address & 0x3) << 3;

  exdf_latch->request.addr = address & ~0x3ULL;
  exdf_latch->request.data = (rt << lshiftamt) >> rshiftamt;
  exdf_latch->request.dqm = (~0U << lshiftamt) >> rshiftamt;
  exdf_latch->request.type = RSP_MEM_REQUEST_WRITE;
  exdf_latch->request.size = request_size;
}

// Function lookup table.
cen64_align(const rsp_function
  rsp_function_table[NUM_RSP_OPCODES], CACHE_LINE_SIZE) = {
#define X(op) op,
#include "rsp/opcodes.md"
#undef X
};

