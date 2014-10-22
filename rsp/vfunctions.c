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
// VAND
//
void RSP_VAND(struct rsp *rsp, uint32_t iw, uint16_t *vd, uint16_t *acc,
  rsp_vect_t vs, rsp_vect_t vt, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vand(vs, vt_shuffle);

  rsp_vect_write_operand(vd, result);
  rsp_vect_write_operand(acc + RSP_ACC_LO, result);
}

//
// VINV
//
void RSP_VINV(struct rsp *rsp, uint32_t iw, uint16_t *vd, uint16_t *acc,
  rsp_vect_t vs, rsp_vect_t vt, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
}

//
// VNAND
//
void RSP_VNAND(struct rsp *rsp, uint32_t iw, uint16_t *vd, uint16_t *acc,
  rsp_vect_t vs, rsp_vect_t vt, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vnand(vs, vt_shuffle, zero);

  rsp_vect_write_operand(vd, result);
  rsp_vect_write_operand(acc + RSP_ACC_LO, result);
}

//
// VNOR
//
void RSP_VNOR(struct rsp *rsp, uint32_t iw, uint16_t *vd, uint16_t *acc,
  rsp_vect_t vs, rsp_vect_t vt, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vnor(vs, vt_shuffle, zero);

  rsp_vect_write_operand(vd, result);
  rsp_vect_write_operand(acc + RSP_ACC_LO, result);
}

//
// VNXOR
//
void RSP_VNXOR(struct rsp *rsp, uint32_t iw, uint16_t *vd, uint16_t *acc,
  rsp_vect_t vs, rsp_vect_t vt, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vnxor(vs, vt_shuffle, zero);

  rsp_vect_write_operand(vd, result);
  rsp_vect_write_operand(acc + RSP_ACC_LO, result);
}

//
// VOR
//
void RSP_VOR(struct rsp *rsp, uint32_t iw, uint16_t *vd, uint16_t *acc,
  rsp_vect_t vs, rsp_vect_t vt, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vor(vs, vt_shuffle);

  rsp_vect_write_operand(vd, result);
  rsp_vect_write_operand(acc + RSP_ACC_LO, result);
}

//
// VXOR
//
void RSP_VXOR(struct rsp *rsp, uint32_t iw, uint16_t *vd, uint16_t *acc,
  rsp_vect_t vs, rsp_vect_t vt, rsp_vect_t vt_shuffle, rsp_vect_t zero) {
  rsp_vect_t result = rsp_vxor(vs, vt_shuffle);

  rsp_vect_write_operand(vd, result);
  rsp_vect_write_operand(acc + RSP_ACC_LO, result);
}

// Function lookup table.
cen64_align(const rsp_vector_function
  rsp_vector_function_table[NUM_RSP_VECTOR_OPCODES], CACHE_LINE_SIZE) = {
#define X(op) op,
#include "rsp/vector_opcodes.md"
#undef X
};

