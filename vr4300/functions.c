//
// vr4300/functions.c: VR4300 execution functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#define VR4300_BUILD_FUNCS

#include "common.h"
#include "bus/controller.h"
#include "vr4300/cp0.h"
#include "vr4300/cp1.h"
#include "vr4300/cpu.h"
#include "vr4300/decoder.h"
#include "vr4300/opcodes.h"
#include "vr4300/pipeline.h"
#include "vr4300/segment.h"

// Mask to negate second operand if subtract operation.
cen64_align(static const uint64_t vr4300_addsub_lut[2], 16) = {
  0x0ULL, ~0x0ULL
};

// Mask to select outputs for bitwise operations.
cen64_align(static const uint64_t vr4300_bitwise_lut[4][2], 64) = {
  {~0ULL,  0ULL}, // AND
  {~0ULL, ~0ULL}, // OR
  { 0ULL, ~0ULL}, // XOR
  { 0ULL,  0ULL}, // -
};

// Mask to kill the instruction word if "likely" branch.
cen64_align(static const uint32_t vr4300_branch_lut[2], 8) = {
  ~0U, 0U
};

// Mask to selectively sign-extend compute values.
cen64_align(static const uint64_t vr4300_mult_sex_mask[2], 16) = {
  ~0ULL, ~0ULL >> 32
};

// Mask to selectively sign-extend loaded values.
cen64_align(static const uint64_t vr4300_load_sex_mask[2][4], CACHE_LINE_SIZE) = {
  {~0ULL,   ~0ULL,     0ULL, ~0ULL},          // sex
  {0xFFULL, 0xFFFFULL, 0ULL, 0xFFFFFFFFULL},  // zex
};

//
// ADD
// SUB
//
int VR4300_ADD_SUB(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = vr4300_addsub_lut[iw >> 1 & 0x1];

  unsigned dest;
  uint64_t rd;

  dest = GET_RD(iw);
  rt = (rt ^ mask) - mask;
  rd = rs + rt;

  // TODO/FIXME: Uncomment this later...
  //assert(((rd >> 31) == (rd >> 32)) && "Overflow exception.");

  exdc_latch->result = (int32_t) rd;
  exdc_latch->dest = dest;
  return 0;
}

//
// ADDI
// SUBI
//
int VR4300_ADDI_SUBI(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = 0; //vr4300_addsub_lut[iw & 0x1];

  unsigned dest;

  dest = GET_RT(iw);
  rt = (int16_t) iw;
  rt = (rt ^ mask) - mask;
  rt = rs + rt;

  // TODO/FIXME: Uncomment this later...
  //assert(((rt >> 31) == (rt >> 32)) && "Overflow exception.");

  exdc_latch->result = (int32_t) rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// ADDIU
// SUBIU
//
int VR4300_ADDIU_SUBIU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = 0; //vr4300_addsub_lut[iw & 0x1];

  unsigned dest;

  dest = GET_RT(iw);
  rt = (int16_t) iw;
  rt = (rt ^ mask) - mask;
  rt = rs + rt;

  exdc_latch->result = (int32_t) rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// ADDU
// SUBU
//
int VR4300_ADDU_SUBU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = vr4300_addsub_lut[iw >> 1 & 0x1];

  unsigned dest;
  uint64_t rd;

  dest = GET_RD(iw);
  rt = (rt ^ mask) - mask;
  rd = rs + rt;

  exdc_latch->result = (int32_t) rd;
  exdc_latch->dest = dest;
  return 0;
}

//
// AND
// OR
// XOR
//
int VR4300_AND_OR_XOR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t and_mask = vr4300_bitwise_lut[iw & 0x3][0];
  uint64_t xor_mask = vr4300_bitwise_lut[iw & 0x3][1];

  unsigned dest;
  uint64_t rd;

  dest = GET_RD(iw);
  rd = ((rs & rt) & and_mask) | ((rs ^ rt) & xor_mask);

  exdc_latch->result = rd;
  exdc_latch->dest = dest;
  return 0;
}

//
// ANDI
// ORI
// XORI
//
int VR4300_ANDI_ORI_XORI(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t and_mask = vr4300_bitwise_lut[iw >> 26 & 0x3][0];
  uint64_t xor_mask = vr4300_bitwise_lut[iw >> 26 & 0x3][1];

  unsigned dest;

  dest = GET_RT(iw);
  rt = (uint16_t) iw;
  rt = ((rs & rt) & and_mask) | ((rs ^ rt) & xor_mask);

  exdc_latch->result = rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// BEQ
//
// If VR4300_BUSY_WAIT_DETECTOR is defined, this version of BEQ
// is used. Otherwise, the version below is used. The reason we
// have this BEQ is to detect libultra's busy wait loops:
//
// beq $0, $0, -4
// nop
//
// When we detect such loops in cached memory, the pipeline
// simulation mostly shuts down and goes into an accelerated mode
// where fewer checks are made (to boost performance).
//
// XXX: Ensure we're in cached memory AND we're followed by a NOP.
//      If we don't do both of these things, we could get ourselves
//      into trouble.
//
int VR4300_BEQ(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  int64_t offset = (uint64_t) ((int16_t) iw) << 2;

  if (rs != rt)
    return 0;

  icrf_latch->pc = rfex_latch->common.pc + (offset + 4);

  if (icrf_latch->pc == rfex_latch->common.pc &&
    GET_RS(iw) == 0 && GET_RT(iw) == 0) {
    //debug("Enter busy wait @ %llu cycles\n", vr4300->cycles);

    exdc_latch->dest = PIPELINE_CYCLE_TYPE;
    exdc_latch->result = 5;
  }

  return 0;
}

//
// BEQ
// BEQL
// BNE
// BNEL
//
int VR4300_BEQ_BEQL_BNE_BNEL(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  uint32_t mask = vr4300_branch_lut[iw >> 30 & 0x1];
  uint64_t offset = (uint64_t) ((int16_t) iw) << 2;

  bool is_ne = iw >> 26 & 0x1;
  bool cmp = rs == rt;

  if (cmp == is_ne) {
    rfex_latch->iw_mask = mask;
    return 0;
  }

  icrf_latch->pc = rfex_latch->common.pc + (offset + 4);
  return 0;
}

//
// BGEZ
// BGEZL
// BLTZ
// BLTZL
//
int VR4300_BGEZ_BGEZL_BLTZ_BLTZL(
  struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  uint32_t mask = vr4300_branch_lut[iw >> 17 & 0x1];
  uint64_t offset = (uint64_t) ((int16_t) iw) << 2;

  bool is_ge = iw >> 16 & 0x1;
  bool cmp = (int64_t) rs < 0;

  if (cmp == is_ge) {
    rfex_latch->iw_mask = mask;
    return 0;
  }

  icrf_latch->pc = rfex_latch->common.pc + (offset + 4);
  return 0;
}

//
// BGEZAL
// BGEZALL
// BLTZAL
// BLTZALL
//
int VR4300_BGEZAL_BGEZALL_BLTZAL_BLTZALL(
  struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t mask = vr4300_branch_lut[iw >> 17 & 0x1];
  uint64_t offset = (uint64_t) ((int16_t) iw) << 2;

  bool is_ge = iw >> 16 & 0x1;
  bool cmp = (int64_t) rs < 0;

  exdc_latch->result = rfex_latch->common.pc + 8;
  exdc_latch->dest = VR4300_REGISTER_RA;

  if (cmp == is_ge) {
    rfex_latch->iw_mask = mask;
    return 0;
  }

  icrf_latch->pc = rfex_latch->common.pc + (offset + 4);
  return 0;
}

//
// BGTZ
// BGTZL
// BLEZ
// BLEZL
//
int VR4300_BGTZ_BGTZL_BLEZ_BLEZL(
  struct vr4300 *vr4300, uint32_t iw, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  uint32_t mask = vr4300_branch_lut[iw >> 30 & 0x1];
  uint64_t offset = (uint64_t) ((int16_t) iw) << 2;

  bool is_gt = iw >> 26 & 0x1;
  bool cmp = (int64_t) rs <= 0;

  if (cmp == is_gt) {
    rfex_latch->iw_mask = mask;
    return 0;
  }

  icrf_latch->pc = rfex_latch->common.pc + (offset + 4);
  return 0;
}

//
// CACHE
//
int VR4300_CACHE(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  uint32_t cp0_status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  struct vr4300_dcache_line *line;
  const struct segment *segment;

  uint64_t vaddr = rs + (int16_t) iw;
  unsigned code = iw >> 16 & 0x3;
  unsigned op = iw >> 18 & 0x7;
  uint32_t paddr;

  // Look up the segment that we're in.
  if ((segment = get_segment(vaddr, cp0_status)) == NULL)
    abort();

  assert(!segment->mapped);
  paddr = vaddr - segment->offset;

  switch(code) {
    case 0: // Instruction cache
      switch(op) {
        case 2:
          vr4300_icache_set_taglo(&vr4300->icache, vaddr,
            vr4300->regs[VR4300_CP0_REGISTER_TAGLO]);
          break;

        case 4:
          vr4300_icache_invalidate_hit(&vr4300->icache, vaddr, paddr);
          break;

        default:
          debug("Unimplemented ICACHE operation: %u\n", op);
          break;
      }

      break;

    case 1: // Data cache
      switch(op) {
        case 0:
          if ((line = vr4300_dcache_wb_invalidate(&vr4300->dcache, vaddr))) {
            uint32_t bus_address;
            uint32_t data[4];
            unsigned i;

            bus_address = vr4300_dcache_get_tag(line);
            memcpy(data, line->data, sizeof(data));

            for (i = 0; i < 4; i++)
              bus_write_word(vr4300->bus, bus_address + i * 4, data[i], ~0);
          }

          break;

        case 3:
          if ((line = vr4300_dcache_should_flush_line(&vr4300->dcache, vaddr))) {
            uint32_t bus_address;
            uint32_t data[4];
            unsigned i;

            bus_address = vr4300_dcache_get_tag(line);
            memcpy(data, line->data, sizeof(data));

            for (i = 0; i < 4; i++)
              bus_write_word(vr4300->bus, bus_address + i * 4, data[i], ~0);
          }

          vr4300_dcache_create_dirty_exclusive(&vr4300->dcache, vaddr, paddr);
          break;

        case 4:
          if ((line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr)))
            vr4300_dcache_invalidate(line);
          break;

        case 6:
          if ((line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr))) {
            uint32_t bus_address;
            uint32_t data[4];
            unsigned i;

            if (line->metadata & 0x2) {
              bus_address = vr4300_dcache_get_tag(line);
              memcpy(data, line->data, sizeof(data));

              for (i = 0; i < 4; i++)
                bus_write_word(vr4300->bus, bus_address + i * 4, data[i], ~0);
            }
          }

          break;

        default:
          debug("Unimplemented DCACHE operation: %u\n", op);
          break;
      }

      break;

    default:
      break;
  }

  return 0;
}

//
// DADD
// DSUB
//
int VR4300_DADD_DSUB(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = vr4300_addsub_lut[iw >> 1 & 0x1];

  unsigned dest;
  uint64_t rd;

  dest = GET_RD(iw);
  rt = (rt ^ mask) - mask;
  rd = rs + rt;

  // TODO/FIXME: Uncomment this later...
  //assert(((rd >> 63) == (rd >> 64)) && "Overflow exception.");

  exdc_latch->result = (int64_t) rd;
  exdc_latch->dest = dest;
  return 0;
}

//
// DADDI
// DSUBI
//
int VR4300_DADDI_DSUBI(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = 0; //vr4300_addsub_lut[iw & 0x1];

  unsigned dest;

  dest = GET_RT(iw);
  rt = (int16_t) iw;
  rt = (rt ^ mask) - mask;
  rt = rs + rt;

  // TODO/FIXME: Uncomment this later...
  //assert(((rt >> 63) == (rt >> 64)) && "Overflow exception.");

  exdc_latch->result = (int64_t) rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// DADDIU
// DSUBIU
//
int VR4300_DADDIU_DSUBIU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = 0; //vr4300_addsub_lut[iw & 0x1];

  unsigned dest;

  dest = GET_RT(iw);
  rt = (int16_t) iw;
  rt = (rt ^ mask) - mask;
  rt = rs + rt;

  exdc_latch->result = (int64_t) rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// DADDU
// DSUBU
//
int VR4300_DADDU_DSUBU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = vr4300_addsub_lut[iw >> 1 & 0x1];

  unsigned dest;
  uint64_t rd;

  dest = GET_RD(iw);
  rt = (rt ^ mask) - mask;
  rd = rs + rt;

  exdc_latch->result = (int64_t) rd;
  exdc_latch->dest = dest;
  return 0;
}

//
// DIV
// DIVU
//
int VR4300_DIV_DIVU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  bool is_divu = iw & 0x1;

  uint64_t sex_mask = vr4300_mult_sex_mask[is_divu];
  int64_t rs_sex = (int32_t) rs & sex_mask;
  int64_t rt_sex = (int32_t) rt & sex_mask;

  if (likely(rt_sex != 0)) {
    int32_t div = rs_sex / rt_sex;
    int32_t mod = rs_sex % rt_sex;

    // TODO: Delay the output a few cycles.
    vr4300->regs[VR4300_REGISTER_LO] = div;
    vr4300->regs[VR4300_REGISTER_HI] = mod;
  }

  return 0;
}

//
// DDIV
//
int VR4300_DDIV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t _rs, uint64_t _rt) {
  if (likely(_rt != 0)) {
    int64_t rs = _rs;
    int64_t rt = _rt;

    uint64_t div = rs / rt;
    uint64_t mod = rs % rt;

    // TODO: Delay the output a few cycles.
    vr4300->regs[VR4300_REGISTER_LO] = div;
    vr4300->regs[VR4300_REGISTER_HI] = mod;
  }

  return 0;
}

//
// DDIVU
//
int VR4300_DDIVU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  if (likely(rt != 0)) {
    uint64_t div = rs / rt;
    uint64_t mod = rs % rt;

    // TODO: Delay the output a few cycles.
    vr4300->regs[VR4300_REGISTER_LO] = div;
    vr4300->regs[VR4300_REGISTER_HI] = mod;
  }

  return 0;
}

//
// DMULT
//
int VR4300_DMULT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  int64_t lo, hi;

#if defined(__GNUC__) && defined(__x86_64__)
  __uint128_t rsx = (uint64_t) rs;
  __uint128_t rtx = (uint64_t) rt;
  __uint128_t result;

  result = rsx * rtx;

  lo = result;
  hi = result >> 64;
#else
  int64_t hi_prod, mid_prods, lo_prod;
  int64_t rshi = (int32_t) (rs >> 32);
  int64_t rthi = (int32_t) (rt >> 32);
  int64_t rslo = (uint32_t) rs;
  int64_t rtlo = (uint32_t) rt;

  mid_prods = (rshi * rtlo) + (rslo * rthi);
  lo_prod = (rslo * rtlo);
  hi_prod = (rshi * rthi);

  mid_prods += lo_prod >> 32;
  hi = hi_prod + (mid_prods >> 32);
  lo = (uint32_t) lo_prod + (mid_prods << 32);
#endif

  // TODO: Delay the output a few cycles.
  vr4300->regs[VR4300_REGISTER_LO] = lo;
  vr4300->regs[VR4300_REGISTER_HI] = hi;
  return 0;
}

//
// DMULTU
//
// TODO: Add a version that works on MSVC.
//
int VR4300_DMULTU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  uint64_t lo, hi;

#if defined(__GNUC__) && defined(__x86_64__)
  __uint128_t rsx = rs;
  __uint128_t rtx = rt;
  __uint128_t result;

  result = rsx * rtx;

  lo = result;
  hi = result >> 64;
#else
  uint64_t hi_prod, mid_prods, lo_prod;
  uint64_t rshi = (int32_t) (rs >> 32);
  uint64_t rthi = (int32_t) (rt >> 32);
  uint64_t rslo = (uint32_t) rs;
  uint64_t rtlo = (uint32_t) rt;

  mid_prods = (rshi * rtlo) + (rslo * rthi);
  lo_prod = (rslo * rtlo);
  hi_prod = (rshi * rthi);

  mid_prods += lo_prod >> 32;
  hi = hi_prod + (mid_prods >> 32);
  lo = (uint32_t) lo_prod + (mid_prods << 32);
#endif

  // TODO: Delay the output a few cycles.
  vr4300->regs[VR4300_REGISTER_LO] = lo;
  vr4300->regs[VR4300_REGISTER_HI] = hi;
  return 0;
}

//
// DSLL
// DSLL32
//
int VR4300_DSLL_DSLL32(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = (iw >> 6 & 0x1F) + ((iw & 0x4) << 3);

  exdc_latch->result = rt << sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// DSLLV
//
int VR4300_DSLLV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x3F;

  exdc_latch->result = rt << sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// DSRA
// DSRA32
//
int VR4300_DSRA_DSRA32(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = (iw >> 6 & 0x1F) + ((iw & 0x4) << 3);

  exdc_latch->result = (int64_t) rt >> sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// DSRAV
//
int VR4300_DSRAV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x3F;

  exdc_latch->result = (int64_t) rt >> sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// DSRL
// DSRL32
//
int VR4300_DSRL_DSRL32(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = (iw >> 6 & 0x1F) + ((iw & 0x4) << 3);

  exdc_latch->result = rt >> sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// DSRLV
//
int VR4300_DSRLV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x3F;

  exdc_latch->result = rt >> sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// INV
//
int VR4300_INV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t unused(rs), uint64_t unused(rt)) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;

  debug("Unimplemented instruction: %s [0x%.8X] @ 0x%.16llX\n",
    vr4300_opcode_mnemonics[rfex_latch->opcode.id], iw, (long long unsigned)
    rfex_latch->common.pc);

  return 0;
}

//
// J
//
// If VR4300_BUSY_WAIT_DETECTOR is defined, this version of J
// is used. Otherwise, the version below is used. The reason we
// have this J is to detect libultra's busy wait loops:
//
// busy_wait:
//   j busy_wait
//   nop
//
// When we detect such loops in cached memory, the pipeline
// simulation mostly shuts down and goes into an accelerated mode
// where fewer checks are made (to boost performance).
//
// XXX: Ensure we're in cached memory AND we're followed by a NOP.
//      If we don't do both of these things, we could get ourselves
//      into trouble.
//
int VR4300_J(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t target = iw << 2 & 0x0FFFFFFF;

  icrf_latch->pc = (rfex_latch->common.pc & ~0x0FFFFFFFULL) | target;

  if (icrf_latch->pc == rfex_latch->common.pc) {
    //debug("Enter busy wait @ %llu cycles\n", vr4300->cycles);

    exdc_latch->dest = PIPELINE_CYCLE_TYPE;
    exdc_latch->result = 5;
  }

  return 0;
}

//
// J
// JAL
//
int VR4300_J_JAL(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  bool is_jal = iw >> 26 & 0x1;
  uint32_t target = iw << 2 & 0x0FFFFFFF;
  uint32_t mask = vr4300_branch_lut[is_jal];

  exdc_latch->result = rfex_latch->common.pc + 8;
  exdc_latch->dest = VR4300_REGISTER_RA & ~mask;

  icrf_latch->pc = (rfex_latch->common.pc & ~0x0FFFFFFFULL) | target;
  return 0;
}

//
// JALR
// JR
//
int VR4300_JALR_JR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  bool is_jalr = iw & 0x1;
  uint32_t mask = vr4300_branch_lut[is_jalr];

  exdc_latch->result = rfex_latch->common.pc + 8;
  exdc_latch->dest = VR4300_REGISTER_RA & ~mask;

  icrf_latch->pc = rs;
  return 0;
}

//
// LD
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LD(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RT(iw);

  exdc_latch->request.vaddr = rs + (int16_t) iw;
  exdc_latch->request.dqm = ~0ULL;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.two_words = 1;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = 8;

  exdc_latch->dest = dest;
  exdc_latch->result = 0;
  return 0;
}

//
// LB
// LBU
// LH
// LHU
// LW
// LWU
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LOAD(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned request_size = (iw >> 26 & 0x3);
  uint64_t dqm = vr4300_load_sex_mask[iw >> 28 & 0x1][request_size];
  unsigned dest = GET_RT(iw);

  exdc_latch->request.vaddr = rs + (int16_t) iw;
  exdc_latch->request.dqm = dqm;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.two_words = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = request_size + 1;

  exdc_latch->dest = dest;
  exdc_latch->result = 0;
  return 0;
}

//
// LUI
//
int VR4300_LUI(struct vr4300 *vr4300,
  uint32_t iw, uint64_t unused(rs), uint64_t unused(rt)) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  int32_t imm = iw << 16;
  unsigned dest = GET_RT(iw);

  exdc_latch->result = imm;
  exdc_latch->dest = dest;
  return 0;
}

//
// LDL
// LDR
//
int VR4300_LDL_LDR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint64_t address = rs + (int16_t) iw;
  unsigned offset = address & 0x7;
  unsigned dest = GET_RT(iw);
  uint64_t dqm;
  int size;

  // LDR
  if (iw >> 26 & 0x1) {
    address ^= offset;
    size = offset + 1;
    dqm = ~0ULL >> ((8 - size) << 3);
  }

  // LDL
  else {
    size = 8;
    dqm = ~0ULL << (offset << 3);
  }

  exdc_latch->request.vaddr = address;
  exdc_latch->request.dqm = dqm;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.two_words = 1;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = size;

  exdc_latch->dest = dest;
  exdc_latch->result = rt & ~dqm;
  return 0;
}

//
// LWL
// LWR
//
int VR4300_LWL_LWR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint64_t address = rs + (int16_t) iw;
  unsigned offset = address & 0x3;
  unsigned dest = GET_RT(iw);
  uint64_t dqm;
  int size;

  // LWR
  if (iw >> 28 & 0x1) {
    cen64_align(static const uint64_t forceset[], 32) =
      {0ULL, 0ULL, 0ULL, ~0ULL};

    size = offset + 1;
    dqm = ~0U >> ((4 - size) << 3);
    address ^= offset;

    //
    // TODO/FIXME: Assume 32-bit mode.
    //
    dqm |= forceset[offset];
  }

  // LWL
  else {
    dqm = ~0ULL << (offset << 3);
    size = 4;
  }

  exdc_latch->request.vaddr = address;
  exdc_latch->request.dqm = dqm;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.two_words = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = size;

  exdc_latch->dest = dest;
  exdc_latch->result = rt & ~dqm;
  return 0;
}

//
// MFHI
// MFLO
//
int VR4300_MFHI_MFLO(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  bool is_mflo = iw >> 1 & 0x1;

  // TODO: Read these here, or...? Registers are probably tied into EX logic...
  exdc_latch->result = vr4300->regs[VR4300_REGISTER_HI + is_mflo];
  exdc_latch->dest = dest;
  return 0;
}

//
// MTHI
// MTLO
//
int VR4300_MTHI_MTLO(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {

  bool is_mtlo = iw >> 1 & 0x1;

  // TODO: Write these here, or...? Registers are probably tied into EX logic...
  vr4300->regs[VR4300_REGISTER_HI + is_mtlo] = rs;
  return 0;
}

//
// MULT
// MULTU
//
int VR4300_MULT_MULTU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  bool is_multu = iw & 0x1;

  uint64_t sex_mask = vr4300_mult_sex_mask[is_multu];
  uint64_t rs_sex = (int32_t) rs & sex_mask;
  uint64_t rt_sex = (int32_t) rt & sex_mask;
  uint64_t result = rs_sex * rt_sex;

  // TODO: Delay the output a few cycles.
  vr4300->regs[VR4300_REGISTER_LO] = (int32_t) result;
  vr4300->regs[VR4300_REGISTER_HI] = (int32_t) (result >> 32);
  return 0;
}

//
// NOR
//
int VR4300_NOR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);

  exdc_latch->result = ~(rs | rt);
  exdc_latch->dest = dest;
  return 0;
}

//
// SD
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_SD(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.vaddr = rs + (int16_t) iw;
  exdc_latch->request.data = rt;
  exdc_latch->request.dqm = ~0ULL;
  exdc_latch->request.size = 8;
  return 0;
}

//
// SLL
//
int VR4300_SLL(struct vr4300 *vr4300,
  uint32_t iw, uint64_t unused(rs), uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = iw >> 6 & 0x1F;

  exdc_latch->result = (int32_t) (rt << sa);
  exdc_latch->dest = dest;
  return 0;
}

//
// SLLV
//
int VR4300_SLLV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x1F;

  exdc_latch->result = (int32_t) (rt << sa);
  exdc_latch->dest = dest;
  return 0;
}

//
// SLT
//
int VR4300_SLT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);

  exdc_latch->result = (int64_t) rs < (int64_t) rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// SLTI
//
int VR4300_SLTI(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RT(iw);
  int64_t imm = (int16_t) iw;

  exdc_latch->result = (int64_t) rs < imm;
  exdc_latch->dest = dest;
  return 0;
}

//
// SLTIU
//
int VR4300_SLTIU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RT(iw);
  uint64_t imm = (int16_t) iw;

  exdc_latch->result = rs < imm;
  exdc_latch->dest = dest;
  return 0;
}

//
// SLTU
//
int VR4300_SLTU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);

  exdc_latch->result = rs < rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// SRA
//
int VR4300_SRA(struct vr4300 *vr4300,
  uint32_t iw, uint64_t unused(rs), uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = iw >> 6 & 0x1F;

  exdc_latch->result = (int32_t) rt >> sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// SRAV
//
int VR4300_SRAV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x1F;

  exdc_latch->result = (int32_t) rt >> sa;
  exdc_latch->dest = dest;
  return 0;
}

//
// SRL
//
int VR4300_SRL(struct vr4300 *vr4300,
  uint32_t iw, uint64_t unused(rs), uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = iw >> 6 & 0x1F;

  exdc_latch->result = (int32_t) ((uint32_t) rt >> sa);
  exdc_latch->dest = dest;
  return 0;
}

//
// SRLV
//
int VR4300_SRLV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);
  unsigned sa = rs & 0x1F;

  exdc_latch->result = (int32_t) ((uint32_t) rt >> sa);
  exdc_latch->dest = dest;
  return 0;
}

//
// SB
// SH
// SW
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_STORE(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint64_t address = rs + (int16_t) iw;
  unsigned request_size = (iw >> 26 & 0x3) + 1;
  unsigned lshiftamt = (4 - request_size) << 3;
  unsigned rshiftamt = (address & 0x3) << 3;

  exdc_latch->request.vaddr = address & ~0x3ULL;
  exdc_latch->request.data = (rt << lshiftamt) >> rshiftamt;
  exdc_latch->request.dqm = (~0U << lshiftamt) >> rshiftamt;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = request_size;
  return 0;
}

//
// SWL
// SWR
//
int VR4300_SWL_SWR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint64_t address = rs + (int16_t) iw;
  unsigned offset = address & 0x3;
  uint32_t mask = ~0U;

  unsigned shiftamt;
  uint64_t data;
  uint64_t dqm;

  // SWR
  if (iw >> 28 & 0x1) {
    shiftamt = (3 - offset) << 3;
    data = rt << shiftamt;
    dqm = mask << shiftamt;
  }

  // SWL
  else {
    shiftamt = offset << 3;
    data = rt >> shiftamt;
    dqm = mask >> shiftamt;
  }

  exdc_latch->request.vaddr = address & ~0x3ULL;
  exdc_latch->request.data = data;
  exdc_latch->request.dqm = dqm;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = 4;
  return 0;
}

// Function lookup table.
cen64_align(const vr4300_function
  vr4300_function_table[NUM_VR4300_OPCODES], CACHE_LINE_SIZE) = {
#define X(op) op,
#include "vr4300/opcodes.md"
#undef X
};

