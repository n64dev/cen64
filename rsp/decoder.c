//
// rsp/decoder.c: RSP decoder.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#define RSP_BUILD_OP(op, func, flags) \
  (RSP_OPCODE_##op), (flags)

#include "common.h"
#include "rsp/decoder.h"
#include "rsp/opcodes.h"
#include "rsp/opcodes_priv.h"

// ============================================================================
//  Escaped opcode table: Special.
//
//      31---------26-----------------------------------------5---------0
//      | SPECIAL/6 |                                         | OPCODE/6|
//      ------6----------------------------------------------------6-----
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//  000 |  SLL  |  ---  |  SRL  |  SRA  | SLLV  |  ---  | SRLV  | SRAV  |
//  001 |  JR   |  JALR |  ---  |  ---  |  ---  | BREAK |  ---  |  ---  |
//  010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  100 |  ADD  | ADDU  |  SUB  | SUBU  |  AND  |  OR   |  XOR  |  NOR  |
//  101 |  ---  |  ---  |  SLT  | SLTU  |  ---  |  ---  |  ---  |  ---  |
//  110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_spec_opcode_table[64], CACHE_LINE_SIZE) = {
  {SLL},     {INVALID}, {SRL},     {SRA},
  {SLLV},    {INVALID}, {SRLV},    {SRAV},
  {JR},      {JALR},    {INVALID}, {INVALID},
  {INVALID}, {BREAK},   {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {ADDU},    {ADDU},    {SUBU},    {SUBU},
  {AND},     {OR},      {XOR},     {NOR},
  {INVALID}, {INVALID}, {SLT},     {SLTU},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

// ============================================================================
//  Escaped opcode table: RegImm.
//
//      31---------26----------20-------16------------------------------0
//      | REGIMM/6  |          |  FMT/5  |                              |
//      ------6---------------------5------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 | BLTZ  | BGEZ  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   10 |BLTZAL |BGEZAL |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_regimm_opcode_table[32], CACHE_LINE_SIZE) = {
  {BLTZ},    {BGEZ},    {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {BLTZAL},  {BGEZAL},  {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

// ============================================================================
//  Escaped opcode table: COP0.
//
//      31--------26-25------21 ----------------------------------------0
//      |  COP0/6   |  FMT/5  |                                         |
//      ------6----------5-----------------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 | MFC0  |  ---  |  ---  |  ---  | MTC0  |  ---  |  ---  |  ---  |
//   01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_cop0_opcode_table[32], CACHE_LINE_SIZE) = {
  {MFC0},    {INVALID}, {INVALID}, {INVALID},
  {MTC0},    {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

// ============================================================================
//  Escaped opcode table: COP2/1.
//
//      31--------26-25------21 ----------------------------------------0
//      |  COP2/6   |  FMT/5  |                                         |
//      ------6----------5-----------------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 | MFC2  |  ---  | CFC2  |  ---  | MTC2  |  ---  | CTC2  |  ---  |
//   01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   10 | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT |
//   11 | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_cop2_opcode_table_1[16], CACHE_LINE_SIZE) = {
  {MFC2},    {INVALID}, {CFC2},    {INVALID},
  {MTC2},    {INVALID}, {CTC2},    {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
};

// ============================================================================
//  Escaped opcode table: COP2/2.
//
//      31---------26---25------------------------------------5---------0
//      |  = COP2   | 1 |                                     |  FMT/6  |
//      ------6-------1--------------------------------------------6-----
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//  000 | VMULF | VMULU | VRNDP | VMULQ | VMUDL | VMUDM | VMUDN | VMUDH |
//  001 | VMACF | VMACU | VRNDN | VMACQ | VMADL | VMADM | VMADN | VMADH |
//  010 | VADD  | VSUB  |  ---  | VABS  | VADDC | VSUBC |  ---  |  ---  |
//  011 |  ---  |  ---  |  ---  |  ---  |  ---  | VSAR  |  ---  |  ---  |
//  100 |  VLT  |  VEQ  |  VNE  |  VGE  |  VCL  |  VCH  |  VCR  | VMRG  |
//  101 | VAND  | VNAND |  VOR  | VNOR  | VXOR  | VNXOR |  ---  |  ---  |
//  110 | VRCP  | VRCPL | VRCPH | VMOV  | VRSQ  | VRSQL | VRSQH | VNOP  |
//  111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | VNULL |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_cop2_opcode_table_2[64], CACHE_LINE_SIZE) = {
  {VMULF},    {VMULU},    {VRNDP},    {VMULQ},
  {VMUDL},    {VMUDM},    {VMUDN},    {VMUDH},
  {VMACF},    {VMACU},    {VRNDN},    {VMACQ},
  {VMADL},    {VMADM},    {VMADN},    {VMADH},
  {VADD},     {VSUB},     {VINVALID}, {VABS},
  {VADDC},    {VSUBC},    {VINVALID}, {VINVALID},
  {VINVALID}, {VINVALID}, {VINVALID}, {VINVALID},
  {VINVALID}, {VSAR},     {VINVALID}, {VINVALID},
  {VLT},      {VEQ},      {VNE},      {VGE},
  {VCL},      {VCH},      {VCR},      {VMRG},
  {VAND},     {VNAND},    {VOR},      {VNOR},
  {VXOR},     {VNXOR},    {VINVALID}, {VINVALID},
  {VRCP},     {VRCPL},    {VRCPH},    {VMOV},
  {VRSQ},     {VRSQL},    {VRSQH},    {VNOP},
  {VINVALID}, {VINVALID}, {VINVALID}, {VINVALID},
  {VINVALID}, {VINVALID}, {VINVALID}, {VNULL},
};

// ============================================================================
//  Escaped opcode table: LWC2.
//
//      31---------26-------------------15-------11---------------------0
//      |   LWC2/6  |                   | FUNC/5 |                      |
//      ------6-----------------------------5----------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 |  LBV  |  LSV  |  LLV  |  LDV  |  LQV  |  LRV  |  LPV  |  LUV  |
//   01 |  LHV  |  LFV  |  ---  |  LTV  |  ---  |  ---  |  ---  |  ---  |
//   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_lwc_opcode_table[32], CACHE_LINE_SIZE) = {
  {LBV},     {LSV},     {LLV},     {LDV},
  {LQV},     {LRV},     {LPV},     {LUV},
  {LHV},     {LFV},     {INVALID}, {LTV},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

// ============================================================================
//  Escaped opcode table: SWC2.
//
//      31---------26-------------------15-------11---------------------0
//      |   SWC2/6  |                   | FMT/5  |                      |
//      ------6-----------------------------5----------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//   00 |  SBV  |  SSV  |  SLV  |  SDV  |  SQV  |  SRV  |  SPV  |  SUV  |
//   01 |  SHV  |  SFV  |  SWV  |  STV  |  ---  |  ---  |  ---  |  ---  |
//   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
//
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_swc_opcode_table[32], CACHE_LINE_SIZE) = {
  {SBV},     {SSV},     {SLV},     {SDV},
  {SQV},     {SRV},     {SPV},     {SUV},
  {SHV},     {SFV},     {SWV},     {STV},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

// ============================================================================
//  First-order opcode table.
//
//  0b000000   => Lookup in rsp_spec_opcode_table.
//  0b000001   => Lookup in rsp_regimm_opcode_table.
//  0b010000   => Lookup in rsp_cop0_opcode_table.
//  0b010001   => Lookup in rsp_cop2_opcode_table.
//  0b110010   => Lookup in rsp_lwc_opcode_table.
//  0b111010   => Lookup in rsp_swc_opcode_table.
//
//      31---------26---------------------------------------------------0
//      |  OPCODE/6 |                                                   |
//      ------6----------------------------------------------------------
//      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//  000 | *SPEC | *RGIM |   J   |  JAL  |  BEQ  |  BNE  | BLEZ  | BGTZ  |
//  001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  |  ORI  | XORI  |  LUI  |
//  010 | *COP0 |  ---  | *COP2 |  ---  |  ---  |  ---  |  ---  |  ---  |
//  011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  100 |  LB   |  LH   |  ---  |  LW   |  LBU  |  LHU  |  ---  |  ---  |
//  101 |  SB   |  SH   |  ---  |  SW   |  ---  |  ---  |  ---  |  ---  |
//  110 |  ---  |  ---  | *LWC2 |  ---  |  ---  |  ---  |  ---  |  ---  |
//  111 |  ---  |  ---  | *SWC2 |  ---  |  ---  |  ---  |  ---  |  ---  |
//      |-------|-------|-------|-------|-------|-------|-------|-------|
// ============================================================================
cen64_align(static const struct rsp_opcode
  rsp_opcode_table[64], CACHE_LINE_SIZE) = {
  {INVALID}, {INVALID}, {J},       {JAL},
  {BEQ},     {BNE},     {BLEZ},    {BGTZ},
  {ADDIU},   {ADDIU},   {SLTI},    {SLTIU},
  {ANDI},    {ORI},     {XORI},    {LUI},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {LB},      {LH},      {INVALID}, {LW},
  {LBU},     {LHU},     {INVALID}, {INVALID},
  {SB},      {SH},      {INVALID}, {SW},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

struct rsp_opcode_escape {
  const struct rsp_opcode *table;
  unsigned shift, mask;
};

// Escaped table listings. Most of these will never
// see a processor cache line, so not much waste here.
cen64_align(static const struct rsp_opcode_escape
  rsp_escape_table[128], CACHE_LINE_SIZE) = {
 {rsp_spec_opcode_table,    0, 0x3F}, {rsp_spec_opcode_table,    0, 0x3F},
 {rsp_regimm_opcode_table, 16, 0x1F}, {rsp_regimm_opcode_table, 16, 0x1F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},

 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},

 {rsp_cop0_opcode_table,   21, 0x1F}, {rsp_cop0_opcode_table,   21, 0x1F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_cop2_opcode_table_1, 21, 0x1F}, {rsp_cop2_opcode_table_2,  0, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},

 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},

 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},

 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_lwc_opcode_table,    11, 0x1F}, {rsp_lwc_opcode_table,    11, 0x1F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},

 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_swc_opcode_table,    11, 0x1F}, {rsp_swc_opcode_table,    11, 0x1F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
 {rsp_opcode_table,        26, 0x3F}, {rsp_opcode_table,        26, 0x3F},
};

// Decodes an instruction word.
const struct rsp_opcode* rsp_decode_instruction(uint32_t iw) {
  const struct rsp_opcode_escape *escape = rsp_escape_table + (iw >> 25);
  unsigned index = iw >> escape->shift & escape->mask;
  return escape->table + index;
}

