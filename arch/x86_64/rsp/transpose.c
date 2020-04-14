//
// arch/x86_64/rsp/transpose.c
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rsp/cpu.h"
#include "rsp/rsp.h"

void rsp_ltv(struct rsp *rsp, uint32_t addr, unsigned element, unsigned vt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  for(int i = 0; i < 8; i++){
    uint16_t slice;

    memcpy(&slice, rsp->mem + addr + (i << 1), sizeof(slice));
    slice = byteswap_16(slice);

    rsp->cp2.regs[vt + i].e[(i -  element) & 7] = slice;
  }
}

void rsp_stv(struct rsp *rsp, uint32_t addr, unsigned element, unsigned vt) {
  struct rsp_exdf_latch *exdf_latch = &rsp->pipeline.exdf_latch;

  for(int i = 0; i < 8; i++){
    uint16_t slice = rsp->cp2.regs[vt + ((i + element) & 7)].e[i];
    slice = byteswap_16(slice);

    memcpy(rsp->mem + addr + (i << 1), &slice, sizeof(slice));
  }
}


