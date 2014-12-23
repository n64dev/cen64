//
// rsp/cp2.c: RSP control coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rsp/cpu.h"
#include "rsp/rsp.h"

//
// CFC2
//
void RSP_CFC2(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  struct rsp_cp2 *cp2 = &rsp->cp2;
  unsigned rd, dest;
  uint32_t data;

  dest = GET_RT(iw);
  rd = GET_RD(iw);

  switch (rd & 0x3) {
    case 0: data = rsp_get_vco(cp2->vco); break;
    case 1: data = rsp_get_vcc(cp2->vcc); break;
    case 2: data = rsp_get_vce(cp2->vce); break;
    case 3: data = rsp_get_vce(cp2->vce); break;
  }

  exdf_latch->result.result = data;
  exdf_latch->result.dest = dest;
}

// Initializes the coprocessor.
void rsp_cp2_init(struct rsp *rsp) {
  rsp->cp2.sp_flag = 1;
}

