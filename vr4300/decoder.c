//
// vr4300/decoder.c: VR4300 decoder.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/decoder.h"
#include "vr4300/opcodes.h"

// ============================================================================
//  Escaped opcode table: Special.
//
//      31---------26------------------------------------------5--------0
//      | SPECIAL/6 |                                         |  FMT/6  |
//      ------6----------------------------------------------------6-----
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//  000 | SLL   |       | SRL   | SRA   | SLLV  |       | SRLV  | SRAV  |
//  001 | JR    | JALR  |       |       |SYSCALL| BREAK |       | SYNC  |
//  010 | MFHI  | MTHI  | MFLO  | MTLO  | DSLLV |       | DSRLV | DSRAV |
//  011 | MULT  | MULTU | DIV   | DIVU  | DMULT | DMULTU| DDIV  | DDIVU |
//  100 | ADD   | ADDU  | SUB   | SUBU  | AND   | OR    | XOR   | NOR   |
//  101 |       |       | SLT   | SLTU  | DADD  | DADDU | DSUB  | DSUBU |
//  110 | TGE   | TGEU  | TLT   | TLTU  | TEQ   |       | TNE   |       |
//  111 | DSLL  |       | DSRL  | DSRA  |DSLL32 |       |DSRL32 |DSRA32 |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct vr4300_opcode
  vr4300_spec_opcode_table[64], CACHE_LINE_SIZE) = {
  {SLL},     {INVALID}, {SRL},     {SRA},
  {SLLV},    {INVALID}, {SRLV},    {SRAV},
  {JR},      {JALR},    {INVALID}, {INVALID},
  {SYSCALL}, {BREAK},   {INVALID}, {SYNC},
  {MFHI},    {MTHI},    {MFLO},    {MTLO},
  {DSLLV},   {INVALID}, {DSRLV},   {DSRAV},
  {MULT},    {MULTU},   {DIV},     {DIVU},
  {DMULT},   {DMULTU},  {DDIV},    {DDIVU},
  {ADD},     {ADDU},    {SUB},     {SUBU},
  {AND},     {OR},      {XOR},     {NOR},
  {INVALID}, {INVALID}, {SLT},     {SLTU},
  {DADD},    {DADDU},   {DSUB},    {DSUBU},
  {TGE},     {TGEU},    {TLT},     {TLTU},
  {TEQ},     {INVALID}, {TNE},     {INVALID},
  {DSLL},    {INVALID}, {DSRL},    {DSRA},
  {DSLL32},  {INVALID}, {DSRL32},  {DSRA32}
};

// ============================================================================
//  Escaped opcode table: RegImm.
//
//      31---------26----------20-------16------------------------------0
//      | = REGIMM  |          |  FMT/5  |                              |
//      ------6---------------------5------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 | BLTZ  | BGEZ  | BLTZL | BGEZL |       |       |       |       |
//   01 | TGEI  | TGEIU | TLTI  | TLTIU | TEQI  |       | TNEI  |       |
//   10 | BLTZAL| BGEZAL|BLTZALL|BGEZALL|       |       |       |       |
//   11 |       |       |       |       |       |       |       |       |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct vr4300_opcode
  vr4300_regimm_opcode_table[32], CACHE_LINE_SIZE) = {
  {BLTZ},    {BGEZ},    {BLTZL},   {BGEZL},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {TGEI},    {TGEIU},   {TLTI},    {TLTIU},
  {TEQI},    {INVALID}, {TNEI},    {INVALID},
  {BLTZAL},  {BGEZAL},  {BLTZALL}, {BGEZALL},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

// ============================================================================
//  Escaped opcode table: COP0/1.
//
//      31--------26-25--24-----21--------------------------------------0
//      |   COP0/6  | 0 |  FMT/4  |                                     |
//      ------6-------1------4------------------------------------------0
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 | MFC0  | DMFC0 | CFC0  |  ---  | MTC0  | DMTC0 | CTC0  |  ---  |
//   01 |  BC0  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   10 |  TLB  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
// ============================================================================
cen64_align(static const struct vr4300_opcode
  vr4300_cop0_opcode_table_1[16], CACHE_LINE_SIZE) = {
  {MFC0},    {DMFC0},   {CFC0},    {INVALID},
  {MTC0},    {DMTC0},   {CTC0},    {INVALID},
  {BC0},     {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
};

// ============================================================================
//  Escaped opcode table: COP0/2.
//
//      31--------26-25 -24-----------------------------------5---------0
//      |   COP0/6  | 1 |                                     |  FMT/6  |
//      ------6-------1--------------------------------------------6-----
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//  000 |  ---  | TLBR  | TLBWI |  ---  |  ---  |  ---  | TLBWR |  ---  |
//  001 | TLBP  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  011 | ERET  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  100 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
// ========================================================================= */
cen64_align(static const struct vr4300_opcode
  vr4300_cop0_opcode_table_2[64], CACHE_LINE_SIZE) = {
  {INVALID}, {TLBR},    {TLBWI},   {INVALID},
  {INVALID}, {INVALID}, {TLBWR},   {INVALID},
  {TLBP},    {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {ERET},    {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
};

// ============================================================================
//  Escaped opcode table: COP1/1.
//
//      31--------26-25------21 ----------------------------------------0
//      |  COP1/6   |  FMT/5  |                                         |
//      ------6----------5-----------------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 | MFC1  | DMFC1 | CFC1  |  ---  | MTC1  | DMTC1 | CTC1  |  ---  |
//   01 |  BC1  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   10 | FPUS  | FPUD  |  ---  |  ---  | FPUW  | FPUL  |  ---  |  ---  |
//   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
// ============================================================================
cen64_align(static const struct vr4300_opcode
  vr4300_cop1_opcode_table_1[16], CACHE_LINE_SIZE) = {
  {MFC1},    {DMFC1},   {CFC1},    {INVALID},
  {MTC1},    {DMTC1},   {CTC1},    {INVALID},
  {BC1},     {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
};

// ============================================================================
//  Escaped opcode table: COP1/2.
//
//      31--------26-25 -24-----------------------------------5---------0
//      |   COP0/6  | 1 |                                     |  FMT/6  |
//      ------6-------1--------------------------------------------6-----
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//  000 |  ---  | TLBR  | TLBWI |  ---  |  ---  |  ---  | TLBWR |  ---  |
//  001 | TLBP  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  011 | ERET  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  100 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
// ========================================================================= */
cen64_align(static const struct vr4300_opcode
  vr4300_cop1_opcode_table_2[64], CACHE_LINE_SIZE) = {
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
};

// ============================================================================
//  Escaped opcode table: COP2.
//
//      31--------26-25------21 ----------------------------------------0
//      |  COP2/6   |  FMT/5  |                                         |
//      ------6----------5-----------------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 | MFC2  | DMFC2 | CFC2  |  ---  | MTC2  | DMTC2 | CTC2  |  ---  |
//   01 |  BC2  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct vr4300_opcode
  vr4300_cop2_opcode_table[32], CACHE_LINE_SIZE) = {
  {MFC2},    {DMFC2},   {CFC2},    {INVALID},
  {MTC2},    {DMTC2},   {CTC2},    {INVALID},
  {BC2},     {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

// ============================================================================
//  First-order opcode table.
//
//  0b000000   => Lookup in vr4300_spec_opcode_table.
//  0b000001   => Lookup in vr4300_regimm_opcode_table.
//  0b010000   => Lookup in vr4300_cop0_opcode_table.
//  0b010001   => Lookup in vr4300_cop0_opcode_table.
//  0b010010   => Lookup in vr4300_cop2_opcode_table.
//
//      31---------26---------------------------------------------------0
//      |  OPCODE/6 |                                                   |
//      ------6----------------------------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//  000 | *SPEC | *RGIM | J     | JAL   | BEQ   | BNE   | BLEZ  | BGTZ  |
//  001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  | ORI   | XORI  | LUI   |
//  010 | *COP0 | *COP1 | *COP2 |       | BEQL  | BNEL  | BLEZL | BGTZL |
//  011 | DADDI |DADDIU |  LDL  |  LDR  |       |       |       |       |
//  100 | LB    | LH    | LWL   | LW    | LBU   | LHU   | LWR   | LWU   |
//  101 | SB    | SH    | SWL   | SW    | SDL   | SDR   | SWR   | CACHE |
//  110 | LL    | LWC1  | LWC2  |       | LLD   | LDC1  | LDC2  | LD    |
//  111 | SC    | SWC1  | SWC2  |       | SCD   | SDC1  | SDC2  | SD    |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct vr4300_opcode
  vr4300_opcode_table[64], CACHE_LINE_SIZE) = {
  {INVALID}, {INVALID}, {J},       {JAL},
  {BEQ},     {BNE},     {BLEZ},    {BGTZ},
  {ADDI},    {ADDIU},   {SLTI},    {SLTIU},
  {ANDI},    {ORI},     {XORI},    {LUI},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {BEQL},    {BNEL},    {BLEZL},   {BGTZL},
  {DADDI},   {DADDIU},  {LDL},     {LDR},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {LB},      {LH},      {LWL},     {LW},
  {LBU},     {LHU},     {LWR},     {LWU},
  {SB},      {SH},      {SWL},     {SW},
  {SDL},     {SDR},     {SWR},     {CACHE},
  {LL},      {LWC1},    {LWC2},    {INVALID},
  {LLD},     {LDC1},    {LDC2},    {LD},
  {SC},      {SWC1},    {SWC2},    {INVALID},
  {SCD},     {SDC1},    {SDC2},    {SD}
};

struct vr4300_opcode_escape {
  const struct vr4300_opcode *table;
  unsigned shift, mask;
};

// Escaped table listings. Most of these will never
// see a processor cache line, so not much waste here.
cen64_align(static const struct vr4300_opcode_escape
  vr4300_escape_table[128], CACHE_LINE_SIZE) = {
 {vr4300_spec_opcode_table,    0, 0x3F}, {vr4300_spec_opcode_table,    0, 0x3F},
 {vr4300_regimm_opcode_table, 16, 0x1F}, {vr4300_regimm_opcode_table, 16, 0x1F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},

 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},

 {vr4300_cop0_opcode_table_1, 21, 0x0F}, {vr4300_cop0_opcode_table_2,  0, 0x3F},
 {vr4300_cop1_opcode_table_1, 21, 0x0F}, {vr4300_cop1_opcode_table_2,  0, 0x3F},
 {vr4300_cop2_opcode_table,   21, 0x1F}, {vr4300_cop2_opcode_table,   21, 0x1F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},

 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},

 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},

 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
  
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},

 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
 {vr4300_opcode_table,        26, 0x3F}, {vr4300_opcode_table,        26, 0x3F},
};

// Decodes an instruction word.
const struct vr4300_opcode* vr4300_decode_instruction(uint32_t iw) {
  const struct vr4300_opcode_escape *escape = vr4300_escape_table + (iw >> 25);
  unsigned index = iw >> escape->shift & escape->mask;
  return escape->table + index;
}

