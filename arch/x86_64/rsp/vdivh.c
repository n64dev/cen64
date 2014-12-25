//
// arch/x86_64/rsp/vrcp.c
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "common/reciprocal.h"
#include "rsp/cpu.h"
#include "rsp/rsp.h"
#include <string.h>

__m128i rsp_vdivh(struct rsp *rsp,
  unsigned src, unsigned e, unsigned dest, unsigned de) {

  // Get the element from VT.
  rsp->cp2.div_in = rsp->cp2.regs[src].e[e];

  // Write out the upper part of the result.
  rsp->cp2.regs[dest].e[de] = rsp->cp2.div_out;
  return rsp_vect_load_unshuffled_operand(rsp->cp2.regs[dest].e);
}

