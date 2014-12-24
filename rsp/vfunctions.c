//
// rsp/vfunctions.c: RSP vector execution functions.
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
#include "rsp/cpu.h"
#include "rsp/opcodes.h"
#include "rsp/opcodes_priv.h"
#include "rsp/rsp.h"

//
// VABS
//
rsp_vect_t RSP_VABS(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo;

  rsp_vect_t result = rsp_vabs(vs, vt_shuffle, zero, &acc_lo);

  write_acc_lo(acc, acc_lo);
  return result;
}

//
// VADD
//
rsp_vect_t RSP_VADD(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t carry, acc_lo;

  carry = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[1]);

  rsp_vect_t result = rsp_vadd(vs, vt_shuffle, carry, &acc_lo);

  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, acc_lo);
  return result;
}

//
// VADDC
//
rsp_vect_t RSP_VADDC(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t sn;

  rsp_vect_t result = rsp_vaddc(vs, vt_shuffle, zero, &sn);

  //rsp_vect_write_operand(&rsp->cp2.vco[0], zero); // TODO: Confirm.
  rsp_vect_write_operand(&rsp->cp2.vco[1], sn);
  write_acc_lo(acc, result);
  return result;
}

//
// VAND
//
rsp_vect_t RSP_VAND(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vand(vs, vt_shuffle);

  write_acc_lo(acc, result);
  return result;
}

//
// VCH
//
rsp_vect_t RSP_VCH(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t ge, le, sign, eq, vce;

  rsp_vect_t result = rsp_vch(vs, vt_shuffle, zero, &ge, &le, &eq, &sign, &vce);

  rsp_vect_write_operand(&rsp->cp2.vcc[0], ge);
  rsp_vect_write_operand(&rsp->cp2.vcc[1], le);
  rsp_vect_write_operand(&rsp->cp2.vco[0], eq);
  rsp_vect_write_operand(&rsp->cp2.vco[1], sign);
  rsp_vect_write_operand(&rsp->cp2.vce, vce);
  write_acc_lo(acc, result);
  return result;
}

//
// VCL
//
rsp_vect_t RSP_VCL(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t ge, le, eq, sign, vce;

  ge = rsp_vect_load_unshuffled_operand(&rsp->cp2.vcc[0]);
  le = rsp_vect_load_unshuffled_operand(&rsp->cp2.vcc[1]);
  eq = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[0]);
  sign = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[1]);
  vce = rsp_vect_load_unshuffled_operand(&rsp->cp2.vce);

  rsp_vect_t result = rsp_vcl(vs, vt_shuffle, zero, &ge, &le, eq, sign, vce);

  rsp_vect_write_operand(&rsp->cp2.vcc[0], ge);
  rsp_vect_write_operand(&rsp->cp2.vcc[1], le);
  write_acc_lo(acc, result);
  return result;
}

//
// VCR
//
rsp_vect_t RSP_VCR(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t ge, le;

  rsp_vect_t result = rsp_vcr(vs, vt_shuffle, zero, &ge, &le);

  rsp_vect_write_operand(&rsp->cp2.vcc[0], ge);
  rsp_vect_write_operand(&rsp->cp2.vcc[1], le);
  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VEQ
//
rsp_vect_t RSP_VEQ(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t le, eq;

  eq = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[0]);

  rsp_vect_t result = rsp_veq(vs, vt_shuffle, &le, eq);

  rsp_vect_write_operand(&rsp->cp2.vcc[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vcc[1], le);
  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VGE
//
rsp_vect_t RSP_VGE(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t le, eq, sign;

  eq = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[0]);
  sign = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[1]);

  rsp_vect_t result = rsp_vge(vs, vt_shuffle, &le, eq, sign);

  rsp_vect_write_operand(&rsp->cp2.vcc[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vcc[1], le);
  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VINVALID
//
rsp_vect_t RSP_VINVALID(struct rsp *rsp, uint32_t iw,
  rsp_vect_t *acc, rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
#ifndef NDEBUG
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;

  debug("Unimplemented instruction: %s [0x%.8X] @ 0x%.8X\n",
    rsp_vector_opcode_mnemonics[rdex_latch->opcode.id],
    iw, rdex_latch->common.pc);
#endif

  return zero;
}

//
// VLT
//
rsp_vect_t RSP_VLT(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t le, eq, sign;

  eq = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[0]);
  sign = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[1]);

  rsp_vect_t result = rsp_vlt(vs, vt_shuffle, &le, eq, sign);

  rsp_vect_write_operand(&rsp->cp2.vcc[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vcc[1], le);
  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VMACF
//
rsp_vect_t RSP_VMACF(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;
  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmacf(vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADH
//
rsp_vect_t RSP_VMADH(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_md, acc_hi, result;

  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadh(vs, vt_shuffle, zero, &acc_md, &acc_hi);

  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADL
//
rsp_vect_t RSP_VMADL(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadl(vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADM
//
rsp_vect_t RSP_VMADM(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadm(vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADN
//
rsp_vect_t RSP_VMADN(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadn(vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMRG
//
rsp_vect_t RSP_VMRG(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t le;

  le = rsp_vect_load_unshuffled_operand(&rsp->cp2.vcc[1]);

  rsp_vect_t result = rsp_vmrg(vs, vt_shuffle, le);

  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VMUDH
//
rsp_vect_t RSP_VMUDH(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_md, acc_hi, result;

  result = rsp_vmudh(vs, vt_shuffle, &acc_md, &acc_hi);

  write_acc_lo(acc, zero);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMUDL
//
rsp_vect_t RSP_VMUDL(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vmudl(vs, vt_shuffle);

  write_acc_lo(acc, result);
  write_acc_md(acc, zero);
  write_acc_hi(acc, zero);
  return result;
}

//
// VMUDM
//
rsp_vect_t RSP_VMUDM(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  result = rsp_vmudm(vs, vt_shuffle, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMUDN
//
rsp_vect_t RSP_VMUDN(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  result = rsp_vmudn(vs, vt_shuffle, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMULF
//
rsp_vect_t RSP_VMULF(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  result = rsp_vmulf(vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMULU
//
rsp_vect_t RSP_VMULU(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  result = rsp_vmulu(vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VNAND
//
rsp_vect_t RSP_VNAND(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vnand(vs, vt_shuffle, zero);

  write_acc_lo(acc, result);
  return result;
}

//
// VNE
//
rsp_vect_t RSP_VNE(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t le, eq;

  eq = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[0]);

  rsp_vect_t result = rsp_vne(vs, vt_shuffle, zero, &le, eq);

  rsp_vect_write_operand(&rsp->cp2.vcc[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vcc[1], le);
  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VNOR
//
rsp_vect_t RSP_VNOR(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vnor(vs, vt_shuffle, zero);

  write_acc_lo(acc, result);
  return result;
}

//
// VNXOR
//
rsp_vect_t RSP_VNXOR(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vnxor(vs, vt_shuffle, zero);

  write_acc_lo(acc, result);
  return result;
}

//
// VOR
//
rsp_vect_t RSP_VOR(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vor(vs, vt_shuffle);

  write_acc_lo(acc, result);
  return result;
}

//
// VRCP
// VRCPL
//
rsp_vect_t RSP_VRCP(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  unsigned de = GET_DE(iw) & 0x7;
  unsigned e = GET_E(iw) & 0x7;

  unsigned dest = GET_VD(iw);
  unsigned src = GET_VT(iw);

  write_acc_lo(acc, vt_shuffle);

  // Force single precision for VRCP (but not VRCPL).
  int dp = iw & rsp->cp2.dp_flag;
  rsp->cp2.dp_flag = 0;

  return rsp_vrcp(rsp, dp, src, e, dest, de);
}

//
// VRCPH
// VRSQH
//
rsp_vect_t RSP_VRCPH_VRSQH(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  unsigned de = GET_DE(iw) & 0x7;
  unsigned e = GET_E(iw) & 0x7;

  unsigned dest = GET_VD(iw);
  unsigned src = GET_VT(iw);

  write_acc_lo(acc, vt_shuffle);

  // Specify double-precision for VRCPL on the next pass.
  rsp->cp2.dp_flag = 1;

  return rsp_vdivh(rsp, src, e, dest, de);
}

//
// VRSQ
// VRSQL
//
rsp_vect_t RSP_VRSQ(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  unsigned de = GET_DE(iw) & 0x7;
  unsigned e = GET_E(iw) & 0x7;

  unsigned dest = GET_VD(iw);
  unsigned src = GET_VT(iw);

  write_acc_lo(acc, vt_shuffle);

  // Force single precision for VRSQ (but not VRSQL).
  int dp = iw & rsp->cp2.dp_flag;
  rsp->cp2.dp_flag = 0;

  return rsp_vrsq(rsp, dp, src, e, dest, de);
}

//
// VSAR
//
rsp_vect_t RSP_VSAR(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  unsigned e = GET_E(iw);

  switch (e) {
    case 8: return read_acc_hi(acc);
    case 9: return read_acc_md(acc);
    case 10: return read_acc_lo(acc);

    default:
      return zero;
  }

  return zero;
}

//
// VSUB
//
rsp_vect_t RSP_VSUB(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t carry, acc_lo;

  carry = rsp_vect_load_unshuffled_operand(&rsp->cp2.vco[1]);

  rsp_vect_t result = rsp_vsub(vs, vt_shuffle, carry, &acc_lo);

  rsp_vect_write_operand(&rsp->cp2.vco[0], zero);
  rsp_vect_write_operand(&rsp->cp2.vco[1], zero);
  write_acc_lo(acc, acc_lo);
  return result;
}

//
// VSUBC
//
rsp_vect_t RSP_VSUBC(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t eq, sn;

  rsp_vect_t result = rsp_vsubc(vs, vt_shuffle, zero, &eq, &sn);

  rsp_vect_write_operand(&rsp->cp2.vco[0], eq);
  rsp_vect_write_operand(&rsp->cp2.vco[1], sn);
  write_acc_lo(acc, result);
  return result;
}

//
// VXOR
//
rsp_vect_t RSP_VXOR(struct rsp *rsp, uint32_t iw, rsp_vect_t *acc,
  rsp_vect_t vs, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vxor(vs, vt_shuffle);

  write_acc_lo(acc, result);
  return result;
}

// Function lookup table.
cen64_align(const rsp_vector_function
  rsp_vector_function_table[NUM_RSP_VECTOR_OPCODES], CACHE_LINE_SIZE) = {
#define X(op) op,
#include "rsp/vector_opcodes.md"
#undef X
};

