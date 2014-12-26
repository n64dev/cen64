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
  unsigned src, unsigned e, unsigned dest, unsigned de) {
  uint16_t data;

  // Get the element from VT.
  data = rsp->cp2.regs[src].e[e];

  // Write out the upper part of the result.
  rsp->cp2.regs[dest].e[de] = data;
  return rsp_vect_load_unshuffled_operand(rsp->cp2.regs[dest].e);
}

