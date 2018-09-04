//
// arch/x86_64/rsp/vmov.c
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rsp/cpu.h"
#include "rsp/rsp.h"

__m128i rsp_vmov(struct rsp *rsp,
  unsigned src, unsigned e, unsigned dest, rsp_vect_t vt_shuffle) {
  uint16_t data;

  // Copy element into data
  memcpy(&data, (e & 0x7) + (uint16_t *)&vt_shuffle, sizeof(uint16_t));

  printf("src %d dest %d el %x data %x\n", src, dest, e, data);
  fflush(stdout);

  // Write out the upper part of the result.
  rsp->cp2.regs[dest].e[e & 0x7] = data;
  return rsp_vect_load_unshuffled_operand(rsp->cp2.regs[dest].e);
}

