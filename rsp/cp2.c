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
    case 0: data = rsp_get_vco(cp2->vco[1].e, cp2->vco[0].e); break;
    case 1: data = rsp_get_vcc(cp2->vcc[1].e, cp2->vcc[0].e); break;
    case 2: data = rsp_get_vce(cp2->vce.e); break;
    case 3: data = rsp_get_vce(cp2->vce.e); break;
  }

  exdf_latch->result.result = data;
  exdf_latch->result.dest = dest;
}

//
// MFC2
//
void RSP_MFC2(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;
  const uint16_t *e = rsp->cp2.regs[GET_RD(iw)].e;
  unsigned dest, element = GET_EL(iw);
  uint8_t low, high;
  uint32_t data;

  unsigned lo = element >> 1;
  unsigned hi;

  dest = GET_RT(iw);

  if (unlikely(element & 0x1)) {
    hi = (element + 1) >> 1;

    low = e[lo] >> ((element & 0x1) << 3);
    high = e[hi] >> (((element + 1) & 0x1) << 3);
    data = (int16_t) ((high << 8) | low);
  }

  else
    data = (int16_t) e[lo];

  exdf_latch->result.result = data;
  exdf_latch->result.dest = dest;
}

//
// MTC2
//
void RSP_MTC2(struct rsp *rsp,
  uint32_t iw, uint32_t rs, uint32_t rt) {
  uint16_t *e = rsp->cp2.regs[GET_RD(iw)].e;
  unsigned element = GET_EL(iw);

  if (unlikely(element & 0x1)) {
    unsigned lo = element >> 1;
    unsigned hi = (element + 1) >> 1;

    e[lo] = (e[lo] & 0x00FF) | ((rt & 0xFF) << 8);
    e[hi] = (e[hi] & 0xFF00) | (rt >> 8 & 0xFF);
  }

  else
    e[element] = rt;
}

