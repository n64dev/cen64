//
// rsp/vfunctions.c: RSP vector execution functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
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
rsp_vect_t RSP_VABS(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t acc_lo;

  rsp_vect_t result = rsp_vabs(vs, vt_shuffle, zero, &acc_lo);

  write_acc_lo(acc, acc_lo);
  return result;
}

//
// VADD
//
rsp_vect_t RSP_VADD(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t carry, acc_lo;

  carry = read_vco_lo(rsp->cp2.flags[RSP_VCO].e);

  rsp_vect_t result = rsp_vadd(vs, vt_shuffle, carry, &acc_lo);

  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, zero);
  write_acc_lo(acc, acc_lo);
  return result;
}

//
// VADDC
//
rsp_vect_t RSP_VADDC(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t sn;

  rsp_vect_t result = rsp_vaddc(vs, vt_shuffle, zero, &sn);

  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, zero); // TODO: Confirm.
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, sn);
  write_acc_lo(acc, result);
  return result;
}

//
// VAND
// VNAND
//
rsp_vect_t RSP_VAND_VNAND(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;

  rsp_vect_t result = rsp_vand_vnand(iw, vs, vt_shuffle);

  write_acc_lo(acc, result);
  return result;
}

//
// VCH
//
rsp_vect_t RSP_VCH(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t ge, le, sign, eq, vce;

  rsp_vect_t result = rsp_vch(vs, vt_shuffle, zero, &ge, &le, &eq, &sign, &vce);

  write_vcc_hi(rsp->cp2.flags[RSP_VCC].e, ge);
  write_vcc_lo(rsp->cp2.flags[RSP_VCC].e, le);
  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, eq);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, sign);
  write_vce   (rsp->cp2.flags[RSP_VCE].e, vce);
  write_acc_lo(acc, result);
  return result;
}

//
// VCL
//
rsp_vect_t RSP_VCL(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t ge, le, eq, sign, vce;

  ge = read_vcc_hi(rsp->cp2.flags[RSP_VCC].e);
  le = read_vcc_lo(rsp->cp2.flags[RSP_VCC].e);
  eq = read_vco_hi(rsp->cp2.flags[RSP_VCO].e);
  sign = read_vco_lo(rsp->cp2.flags[RSP_VCO].e);
  vce = read_vce(rsp->cp2.flags[RSP_VCE].e);

  rsp_vect_t result = rsp_vcl(vs, vt_shuffle, zero, &ge, &le, eq, sign, vce);

  write_vcc_hi(rsp->cp2.flags[RSP_VCC].e, ge);
  write_vcc_lo(rsp->cp2.flags[RSP_VCC].e, le);
  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vce   (rsp->cp2.flags[RSP_VCE].e, zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VCR
//
rsp_vect_t RSP_VCR(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t ge, le;

  rsp_vect_t result = rsp_vcr(vs, vt_shuffle, zero, &ge, &le);

  write_vcc_hi(rsp->cp2.flags[RSP_VCC].e, ge);
  write_vcc_lo(rsp->cp2.flags[RSP_VCC].e, le);
  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vce   (rsp->cp2.flags[RSP_VCE].e, zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VEQ
// VGE
// VLT
// VNE
//
rsp_vect_t RSP_VEQ_VGE_VLT_VNE(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t le, eq, sign;

  eq = read_vco_hi(rsp->cp2.flags[RSP_VCO].e);
  sign = read_vco_lo(rsp->cp2.flags[RSP_VCO].e);

  rsp_vect_t result = rsp_veq_vge_vlt_vne(iw, vs, vt_shuffle,
    zero, &le, eq, sign);

  write_vcc_hi(rsp->cp2.flags[RSP_VCC].e, zero);
  write_vcc_lo(rsp->cp2.flags[RSP_VCC].e, le);
  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VINVALID
//
rsp_vect_t RSP_VINVALID(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
#ifndef NDEBUG
  struct rsp_rdex_latch *rdex_latch = &rsp->pipeline.rdex_latch;

  debug("Unimplemented instruction: %s [0x%.8X] @ 0x%.8X\n",
    rsp_vector_opcode_mnemonics[rdex_latch->opcode.id],
    iw, rdex_latch->common.pc);
#endif

  return zero;
}

//
// VMACF
// VMACU
//
rsp_vect_t RSP_VMACF_VMACU(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t acc_lo, acc_md, acc_hi, result;
  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmacf_vmacu(iw, vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADH
// VMUDH
//
rsp_vect_t RSP_VMADH_VMUDH(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadh_vmudh(iw, vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADL
// VMUDL
//
rsp_vect_t RSP_VMADL_VMUDL(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadl_vmudl(iw, vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADM
// VMUDM
//
rsp_vect_t RSP_VMADM_VMUDM(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadm_vmudm(iw, vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMADN
// VMUDN
//
rsp_vect_t RSP_VMADN_VMUDN(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  acc_lo = read_acc_lo(acc);
  acc_md = read_acc_md(acc);
  acc_hi = read_acc_hi(acc);

  result = rsp_vmadn_vmudn(iw, vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VMOV
//
rsp_vect_t RSP_VMOV(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  unsigned e = GET_DE(iw) & 0x7;

  unsigned dest = GET_VD(iw);
  unsigned src = GET_VT(iw);

  write_acc_lo(acc, vt_shuffle);

  return rsp_vmov(rsp, src, e, dest, vt_shuffle);
}

//
// VMRG
//
rsp_vect_t RSP_VMRG(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t le;

  le = read_vcc_lo(rsp->cp2.flags[RSP_VCC].e);

  rsp_vect_t result = rsp_vmrg(vs, vt_shuffle, le);

  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, zero);
  write_acc_lo(acc, result);
  return result;
}

//
// VMULF
// VMULU
//
rsp_vect_t RSP_VMULF_VMULU(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t acc_lo, acc_md, acc_hi, result;

  result = rsp_vmulf_vmulu(iw, vs, vt_shuffle, zero, &acc_lo, &acc_md, &acc_hi);

  write_acc_lo(acc, acc_lo);
  write_acc_md(acc, acc_md);
  write_acc_hi(acc, acc_hi);
  return result;
}

//
// VNOP
//
rsp_vect_t RSP_VNOP(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  return vs;
}

//
// VOR
// VNOR
//
rsp_vect_t RSP_VOR_VNOR(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;

  rsp_vect_t result = rsp_vor_vnor(iw, vs, vt_shuffle);

  write_acc_lo(acc, result);
  return result;
}

//
// VRCP
// VRCPL
// VRSQ
// VRSQL
//
rsp_vect_t RSP_VRCP_VRSQ(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  unsigned de = GET_DE(iw) & 0x7;
  unsigned e = GET_E(iw) & 0x7;

  unsigned dest = GET_VD(iw);
  unsigned src = GET_VT(iw);

  write_acc_lo(acc, vt_shuffle);

  // Force single precision for VRCP (but not VRCPL).
  int dp = iw & rsp->cp2.dp_flag;
  rsp->cp2.dp_flag = 0;

  return rsp_vrcp_vrsq(rsp, iw, dp, src, e, dest, de);
}

//
// VRCPH
// VRSQH
//
rsp_vect_t RSP_VRCPH_VRSQH(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
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
// VSAR
//
rsp_vect_t RSP_VSAR(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
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
rsp_vect_t RSP_VSUB(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t carry, acc_lo;

  carry = read_vco_lo(rsp->cp2.flags[RSP_VCO].e);

  rsp_vect_t result = rsp_vsub(vs, vt_shuffle, carry, &acc_lo);

  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, zero);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, zero);
  write_acc_lo(acc, acc_lo);
  return result;
}

//
// VSUBC
//
rsp_vect_t RSP_VSUBC(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;
  rsp_vect_t eq, sn;

  rsp_vect_t result = rsp_vsubc(vs, vt_shuffle, zero, &eq, &sn);

  write_vco_hi(rsp->cp2.flags[RSP_VCO].e, eq);
  write_vco_lo(rsp->cp2.flags[RSP_VCO].e, sn);
  write_acc_lo(acc, result);
  return result;
}

//
// VXOR
// VNXOR
//
rsp_vect_t RSP_VXOR_VNXOR(struct rsp *rsp, uint32_t iw,
  rsp_vect_t vt_shuffle, rsp_vect_t vs, rsp_vect_t zero) {
  uint16_t *acc = rsp->cp2.acc.e;

  rsp_vect_t result = rsp_vxor_vnxor(iw, vs, vt_shuffle);

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

