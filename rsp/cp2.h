//
// rsp/cp2.h: RSP control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __rsp_cp2_h__
#define __rsp_cp2_h__
#include "common.h"
#include "rsp/rsp.h"

enum rsp_acc_t {
  RSP_ACC_LO = 16,
  RSP_ACC_MD = 8,
  RSP_ACC_HI = 0,
};

struct rsp_cp2 {
  rsp_vect_t regs[32];

  rsp_vect_t vcc[2];
  rsp_vect_t vco[2];
  rsp_vect_t vce;

  rsp_vect_t acc[3];
};

void RSP_CFC2(struct rsp *rsp, uint32_t iw, uint32_t rs, uint32_t rt);

#endif

