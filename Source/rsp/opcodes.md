//
// rsp/opcodes.md: RSP opcode types and info.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSP_OPCODE_TABLE
#define RSP_OPCODE_TABLE X(INVALID) \
  X(ADDU) X(ADDIU) X(AND) X(ANDI) X(BEQ) X(BGEZ) X(BGEZAL) X(BGTZ) \
  X(BLEZ) X(BLTZ) X(BLTZAL) X(BNE) X(BREAK) X(CFC2) X(CTC2) X(J) X(JAL) \
  X(JALR) X(JR) X(LB) X(LBU) X(LBV) X(LDV) X(LFV) X(LH) X(LHU) X(LHV) \
  X(LLV) X(LPV) X(LQV) X(LRV) X(LSV) X(LTV) X(LUI) X(LUV) X(LW) X(MFC0) \
  X(MFC2) X(MTC0) X(MTC2) X(NOP) X(NOR) X(OR) X(ORI) X(SB) X(SBV) X(SDV) \
  X(SFV) X(SH) X(SHV) X(SLL) X(SLLV) X(SLT) X(SLTI) X(SLTIU) X(SLTU) \
  X(SLV) X(SPV) X(SQV) X(SRA) X(SRAV) X(SRL) X(SRLV) X(SRV) X(SSV) X(STV) \
  X(SUBU) X(SUV) X(SW) X(SWV) X(XOR) X(XORI)
#endif

RSP_OPCODE_TABLE

