//
// vr4300/functions.c: VR4300 execution functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#define VR4300_BUILD_OP(op, func, flags) \
  (VR4300_##func)

#include "common.h"
#include "bus/controller.h"
#include "vr4300/cp0.h"
#include "vr4300/cp1.h"
#include "vr4300/cpu.h"
#include "vr4300/decoder.h"
#include "vr4300/opcodes.h"
#include "vr4300/opcodes_priv.h"
#include "vr4300/pipeline.h"
#include "vr4300/segment.h"

// Mask to negate second operand if subtract operation.
#if defined(__GNUC__) && defined(__x86_64__)
static inline uint64_t vr4300_addsub_mask(uint32_t iw)
{
  uint64_t mask;
  __asm__("shr $2,       %k[iwiw];"
          "sbb %q[mask], %q[mask];"
    : [mask] "=r" (mask), [iwiw] "+r" (iw) : : "cc");
  return mask;
}
#elif defined(__GNUC__) && defined(__i386__)
static inline uint64_t vr4300_addsub_mask(uint32_t iw)
{
  int32_t mask;
  __asm__("shr $2,       %k[iwiw];"
          "sbb %k[mask], %k[mask];"
    : [mask] "=r" (mask), [iwiw] "+r" (iw) : : "cc");
  return (uint64_t)((int64_t)mask);
}
#else
cen64_align(static const uint64_t vr4300_addsub_lut[4], 32) = {
  0x0ULL, ~0x0ULL, ~0x0ULL, ~0x0ULL
};
static inline uint64_t vr4300_addsub_mask(uint32_t iw)
{
  return vr4300_addsub_lut[iw & 0x2];
}
#endif

// Mask to kill the instruction word if "likely" branch.
static inline uint32_t vr4300_branch_mask(uint32_t iw, unsigned index) {
  iw = (uint32_t)(   (int32_t)(iw << (31 - index)) >> 31  );
  return ~iw; /* ones' complement must be done last on return */
}

// Mask to selectively sign-extend compute values.
cen64_align(static const uint64_t vr4300_mult_sex_mask[2], 16) = {
  ~0ULL, ~0ULL >> 32
};

// Mask to selectively sign-extend loaded values.
cen64_align(static const uint64_t vr4300_load_sex_mask[8], CACHE_LINE_SIZE) = {
  ~0ULL,   ~0ULL,     0ULL, ~0ULL,          // sex
  0xFFULL, 0xFFFFULL, 0ULL, 0xFFFFFFFFULL,  // zex
};

#if defined(__GNUC__) && defined(__x86_64__)
static inline uint64_t VR4300_LWR_forceset(unsigned offset)
{
  uint64_t mask;
  __asm__("add $4294967293, %k[offs];"
          "sbb %q[mask],    %q[mask];"
    : [mask] "=r" (mask), [offs] "+r" (offset) : : "cc");
  return mask;
}
#elif defined(__GNUC__) && defined(__i386__)
static inline uint64_t VR4300_LWR_forceset(unsigned offset)
{
  int32_t mask;
  __asm__("add $4294967293, %k[offs];"
          "sbb %k[mask],    %k[mask];"
    : [mask] "=r" (mask), [offs] "+r" (offset) : : "cc");
  return (uint64_t)((int64_t)mask);
}
#else
cen64_align(static const uint64_t VR4300_LWR_forceset_lut[], 32) =
  {0ULL, 0ULL, 0ULL, ~0ULL};
static inline uint64_t VR4300_LWR_forceset(unsigned offset)
{
  return VR4300_LWR_forceset_lut[offset];
}
#endif

//
// Raises a MCI interlock for a set number of cycles.
//
static inline int vr4300_do_mci(struct vr4300 *vr4300, unsigned cycles) {
  vr4300->pipeline.cycles_to_stall = cycles - 1;
  vr4300->regs[PIPELINE_CYCLE_TYPE] = 3;
  return 1;
}

//
// ADD
// SUB
//
int VR4300_ADD_SUB(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = vr4300_addsub_mask(iw);

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
// LUI
// SUBIU
//
int VR4300_ADDIU_LUI_SUBIU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned immshift = iw >> 24 & 0x10;
  unsigned dest;

  dest = GET_RT(iw);

  rt = (int16_t) iw;
  rt = rs + (rt << immshift);

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
  uint64_t mask = vr4300_addsub_mask(iw);

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

  unsigned dest;
  uint64_t rd, rand, rxor;

  dest = GET_RD(iw);
  rand = rs & rt;
  rxor = rs ^ rt;
  rd = rand + rxor; // lea
  if((iw & 1) == 0) // cmov
    rd = rxor;
  if((iw & 3) == 0) // cmov
    rd = rand;

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

  unsigned dest;
  uint64_t rd, rand, rxor;

  dest = GET_RT(iw);
  rt = (uint16_t) iw;
  rand = rs & rt;
  rxor = rs ^ rt;
  rd = rand + rxor; // lea
  if((iw & 67108864) == 0) // cmov
    rd = rxor;
  if((iw & 201326592) == 0) // cmov
    rd = rand;

  exdc_latch->result = rd;
  exdc_latch->dest = dest;
  return 0;
}

#ifdef VR4300_BUSY_WAIT_DETECTION
//
// BEQ
// BEQL
// BNE
// BNEL
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
int VR4300_BEQ_BEQL_BNE_BNEL_BWDETECT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t mask = vr4300_branch_mask(iw, 30);
  uint64_t offset = (uint64_t) ((int16_t) iw) << 2;

  bool is_ne = iw >> 26 & 0x1;
  bool cmp = rs == rt;

  if (cmp == is_ne) {
    rfex_latch->iw_mask = mask;
    return 0;
  }

  icrf_latch->pc = rfex_latch->common.pc + (offset + 4);

  if (icrf_latch->pc == rfex_latch->common.pc &&
    GET_RS(iw) == 0 && GET_RT(iw) == 0) {
    //debug("Enter busy wait @ %llu cycles\n", vr4300->cycles);

    exdc_latch->dest = PIPELINE_CYCLE_TYPE;
    exdc_latch->result = 5;
  }

  return 0;
}

#else
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
  uint32_t mask = vr4300_branch_mask(iw, 30);
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
#endif

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
  uint32_t mask = vr4300_branch_mask(iw, 17);
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
  uint32_t mask = vr4300_branch_mask(iw, 17);
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
  uint32_t mask = vr4300_branch_mask(iw, 30);
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
cen64_cold static int vr4300_cacheop_unimplemented(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  debug("Unimplemented CACHE operation!\n");

  abort();
  return 0;
}

cen64_cold static int vr4300_cacheop_ic_invalidate(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  vr4300_icache_invalidate(&vr4300->icache, vaddr);

  return 0;
}

cen64_cold static int vr4300_cacheop_ic_set_taglo(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  vr4300_icache_set_taglo(&vr4300->icache, vaddr,
    vr4300->regs[VR4300_CP0_REGISTER_TAGLO]);

  return 0;
}

cen64_cold static int vr4300_cacheop_ic_invalidate_hit(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  vr4300_icache_invalidate_hit(&vr4300->icache, vaddr, paddr);

  return 0;
}

cen64_cold static int vr4300_cacheop_dc_get_taglo(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  vr4300->regs[VR4300_CP0_REGISTER_TAGLO] = (int32_t)
    vr4300_dcache_get_taglo(&vr4300->dcache, vaddr);

  return 0;
}

cen64_cold static int vr4300_cacheop_dc_set_taglo(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  vr4300_dcache_set_taglo(&vr4300->dcache, vaddr,
    vr4300->regs[VR4300_CP0_REGISTER_TAGLO]);

  return 0;
}

cen64_cold static int vr4300_cacheop_dc_wb_invalidate(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line;

  uint32_t bus_address;
  uint32_t data[4];
  unsigned i;

  if (!(line = vr4300_dcache_wb_invalidate(&vr4300->dcache, vaddr)))
    return 0;

  if (line->metadata & 0x2) {
    bus_address = vr4300_dcache_get_tag(line, vaddr);
    memcpy(data, line->data, sizeof(data));

    for (i = 0; i < 4; i++)
      bus_write_word(vr4300->bus, bus_address + i * 4,
        data[i ^ (WORD_ADDR_XOR >> 2)], ~0);

    line->metadata &= ~0x2;
    return DCACHE_ACCESS_DELAY;
  }

  return 0;
}

cen64_cold static int vr4300_cacheop_dc_create_dirty_ex(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line;

  uint32_t bus_address;
  uint32_t data[4];
  unsigned i;

  int delay = 0;

  if ((line = vr4300_dcache_should_flush_line(&vr4300->dcache, vaddr))) {
    bus_address = vr4300_dcache_get_tag(line, vaddr);
    memcpy(data, line->data, sizeof(data));

    for (i = 0; i < 4; i++)
      bus_write_word(vr4300->bus, bus_address + i * 4,
        data[i ^ (WORD_ADDR_XOR >> 2)], ~0);

    delay = DCACHE_ACCESS_DELAY;
  }

  vr4300_dcache_create_dirty_exclusive(&vr4300->dcache, vaddr, paddr);
  return delay;
}

cen64_cold static int vr4300_cacheop_dc_hit_invalidate(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line;

  if ((line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr)))
    vr4300_dcache_invalidate(line);

  return 0;
}

cen64_cold static int vr4300_cacheop_dc_hit_wb_invalidate(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line;

  uint32_t bus_address;
  uint32_t data[4];
  unsigned i;

  if (!(line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr)))
    return 0;

  if (line->metadata & 0x2) {
    bus_address = vr4300_dcache_get_tag(line, vaddr);
    memcpy(data, line->data, sizeof(data));

    for (i = 0; i < 4; i++)
      bus_write_word(vr4300->bus, bus_address + i * 4,
        data[i ^ (WORD_ADDR_XOR >> 2)], ~0);

    line->metadata &= ~0x1;
    return DCACHE_ACCESS_DELAY;
  }

  line->metadata &= ~0x1;
  return 0;
}

cen64_cold static int vr4300_cacheop_dc_hit_wb(
  struct vr4300 *vr4300, uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line;

  uint32_t bus_address;
  uint32_t data[4];
  unsigned i;

  if (!(line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr)))
    return 0;

  if (line->metadata & 0x2) {
    bus_address = vr4300_dcache_get_tag(line, vaddr);
    memcpy(data, line->data, sizeof(data));

    for (i = 0; i < 4; i++)
      bus_write_word(vr4300->bus, bus_address + i * 4,
        data[i ^ (WORD_ADDR_XOR >> 2)], ~0);

    // TODO: Technically, it's clean now...
    line->metadata &= ~0x2;
    return DCACHE_ACCESS_DELAY;
  }

  return 0;
}

int VR4300_CACHE(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {

  cen64_align(static vr4300_cacheop_func_t cacheop_lut[32], CACHE_LINE_SIZE) = {
    vr4300_cacheop_ic_invalidate,     vr4300_cacheop_unimplemented,
    vr4300_cacheop_ic_set_taglo,      vr4300_cacheop_unimplemented,
    vr4300_cacheop_ic_invalidate_hit, vr4300_cacheop_unimplemented,
    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,

    vr4300_cacheop_dc_wb_invalidate,  vr4300_cacheop_dc_get_taglo,
    vr4300_cacheop_dc_set_taglo,      vr4300_cacheop_dc_create_dirty_ex,
    vr4300_cacheop_dc_hit_invalidate, vr4300_cacheop_dc_hit_wb_invalidate,
    vr4300_cacheop_dc_hit_wb,         vr4300_cacheop_unimplemented,

    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,
    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,
    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,
    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,

    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,
    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,
    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented,
    vr4300_cacheop_unimplemented,     vr4300_cacheop_unimplemented 
  };

  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t vaddr = rs + (int16_t) iw;

  uint32_t op_type = (iw >> 18 & 0x7);
  unsigned op = (iw >> 13 & 0x18) | op_type;

  exdc_latch->request.vaddr = vaddr;
  exdc_latch->request.cacheop = cacheop_lut[op];
  exdc_latch->request.type = op_type > 2
    ? VR4300_BUS_REQUEST_CACHE_WRITE
    : VR4300_BUS_REQUEST_CACHE_IDX;

  return 0;
}

//
// DADD
// DSUB
//
int VR4300_DADD_DSUB(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t mask = vr4300_addsub_mask(iw);

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
  uint64_t mask = vr4300_addsub_mask(iw);

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

  int32_t div;
  int32_t mod;
  if (likely(rt_sex != 0)) {
    div = rs_sex / rt_sex;
    mod = rs_sex % rt_sex;
  }
  else {
    div = rs_sex < 0 ? 1 : -1;
    mod = rs_sex;
  }

  // TODO: Delay the output a few cycles.
  vr4300->regs[VR4300_REGISTER_LO] = div;
  vr4300->regs[VR4300_REGISTER_HI] = mod;

  return vr4300_do_mci(vr4300, 37);
}

//
// DDIV
//
int VR4300_DDIV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t _rs, uint64_t _rt) {

  int64_t rs = _rs;
  int64_t rt = _rt;
  uint64_t div;
  uint64_t mod;
  if (unlikely(rs == INT64_MIN && rt == -1)) {
    div = INT64_MIN;
    mod = 0;
  }
  else if (likely(rt != 0)) {
    div = rs / rt;
    mod = rs % rt;
  }
  else {
    div = rs < 0 ? 1 : -1;
    mod = rs;
  }

  // TODO: Delay the output a few cycles.
  vr4300->regs[VR4300_REGISTER_LO] = div;
  vr4300->regs[VR4300_REGISTER_HI] = mod;
  return vr4300_do_mci(vr4300, 69);
}

//
// DDIVU
//
int VR4300_DDIVU(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {

  uint64_t div;
  uint64_t mod;

  if (likely(rt != 0)) {
    div = rs / rt;
    mod = rs % rt;
  }
  else {
    div = -1;
    mod = rs;
  }

  // TODO: Delay the output a few cycles.
  vr4300->regs[VR4300_REGISTER_LO] = div;
  vr4300->regs[VR4300_REGISTER_HI] = mod;

  return vr4300_do_mci(vr4300, 69);
}

//
// DMULT
//
int VR4300_DMULT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  int64_t lo, hi;

#if defined(__GNUC__) && defined(__x86_64__)
  __int128_t rsx = (int64_t) rs;
  __int128_t rtx = (int64_t) rt;
  __int128_t result;

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
  return vr4300_do_mci(vr4300, 8);
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
  return vr4300_do_mci(vr4300, 8);
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
int VR4300_INVALID(struct vr4300 *vr4300,
  uint32_t iw, uint64_t unused(rs), uint64_t unused(rt)) {
#ifndef NDEBUG
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;

  debug("Unimplemented instruction: %s [0x%.8X] @ 0x%.16llX\n",
    vr4300_opcode_mnemonics[rfex_latch->opcode.id], iw, (long long unsigned)
    rfex_latch->common.pc);
#endif

  return 0;
}

#ifdef VR4300_BUSY_WAIT_DETECTION
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
int VR4300_J_JAL_BWDETECT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t target = iw << 2 & 0x0FFFFFFF;
  uint32_t mask = vr4300_branch_mask(iw, 26); // is_jal

  exdc_latch->result = rfex_latch->common.pc + 8;
  exdc_latch->dest = VR4300_REGISTER_RA & ~mask;

  icrf_latch->pc = (rfex_latch->common.pc & ~0x0FFFFFFFULL) | target;

  if (icrf_latch->pc == rfex_latch->common.pc) {
    //debug("Enter busy wait @ %llu cycles\n", vr4300->cycles);

    exdc_latch->dest = PIPELINE_CYCLE_TYPE;
    exdc_latch->result = 5;
  }

  return 0;
}

#else
//
// J
// JAL
//
int VR4300_J_JAL(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t target = iw << 2 & 0x0FFFFFFF;
  uint32_t mask = vr4300_branch_mask(iw, 26); // is_jal

  exdc_latch->result = rfex_latch->common.pc + 8;
  exdc_latch->dest = VR4300_REGISTER_RA & ~mask;

  icrf_latch->pc = (rfex_latch->common.pc & ~0x0FFFFFFFULL) | target;
  return 0;
}
#endif

//
// JALR
// JR
//
int VR4300_JALR_JR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t mask = vr4300_branch_mask(iw, 0); // is_jalr
  uint32_t rd = GET_RD(iw);

  exdc_latch->result = rfex_latch->common.pc + 8;
  exdc_latch->dest = rd & ~mask;

  icrf_latch->pc = rs;
  return 0;
}

//
// LD
// SD
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LD_SD(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t sel_mask = (int64_t) (int32_t) (iw << 2) >> 32;

  exdc_latch->request.vaddr = rs + (int64_t) (int16_t) iw;
  exdc_latch->request.data = ~sel_mask | (sel_mask & rt);
  exdc_latch->request.wdqm = sel_mask;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.access_type = VR4300_ACCESS_DWORD;
  exdc_latch->request.type = 1 - sel_mask;
  exdc_latch->request.size = 8;

  exdc_latch->dest = ~sel_mask & GET_RT(iw);
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
// SB
// SH
// SW
//
// TODO/FIXME: Check for unaligned addresses.
//
cen64_hot int VR4300_LOAD_STORE(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint64_t sel_mask = (int64_t) (int32_t) (iw << 2) >> 32;

  uint64_t address = rs + (int64_t) (int16_t) iw;
  unsigned request_index = (iw >> 26 & 0x7);
  uint64_t dqm = vr4300_load_sex_mask[request_index] & ~sel_mask;
  unsigned request_size = request_index & 0x3;
  unsigned lshiftamt = (3 - request_size) << 3;
  unsigned rshiftamt = (address & 0x3) << 3;

  exdc_latch->request.vaddr = address & ~(sel_mask & 0x3);
  exdc_latch->request.data = dqm | (sel_mask & ((rt << lshiftamt) >> rshiftamt));
  exdc_latch->request.wdqm = ((uint32_t) sel_mask << lshiftamt) >> rshiftamt;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.access_type = VR4300_ACCESS_WORD;
  exdc_latch->request.type = 1 - sel_mask;
  exdc_latch->request.size = request_size + 1;

  exdc_latch->dest = ~sel_mask & GET_RT(iw);
  exdc_latch->result = 0;
  return 0;
}

//
// LDL
// LDR
//
int VR4300_LDL_LDR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint64_t address = rs + (int64_t) (int16_t) iw;
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
  exdc_latch->request.data = dqm;
  exdc_latch->request.wdqm = 0ULL;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.access_type = VR4300_ACCESS_DWORD;
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

  uint64_t address = rs + (int64_t) (int16_t) iw;
  unsigned offset = address & 0x3;
  unsigned dest = GET_RT(iw);
  uint64_t dqm;
  int size;

  // LWR
  if (iw >> 28 & 0x1) {
    size = offset + 1;
    dqm = ~0U >> ((4 - size) << 3);
    address ^= offset;

    //
    // TODO/FIXME: Assume 32-bit mode.
    //

    dqm |= VR4300_LWR_forceset(offset);
  }

  // LWL
  else {
    dqm = ~0ULL << (offset << 3);
    size = 4;
  }

  exdc_latch->request.vaddr = address;
  exdc_latch->request.data = dqm;
  exdc_latch->request.wdqm = 0ULL;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.access_type = VR4300_ACCESS_WORD;
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
  return vr4300_do_mci(vr4300, 57);
}

//
// SLL
// SLLV
//
int VR4300_SLL_SLLV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RD(iw);
  unsigned sa = (rs & 0x1F) + (iw >> 6 & 0x1F);

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
// SDL
// SDR
//
int VR4300_SDL_SDR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint64_t address = rs + (int64_t) (int16_t) iw;
  unsigned offset = address & 0x7;
  uint64_t mask = ~0ULL;

  unsigned shiftamt;
  uint64_t data;
  uint64_t dqm;

  // SDR
  if (iw >> 26 & 0x1) {
    shiftamt = (7 - offset) << 3;
    data = rt << shiftamt;
    dqm = mask << shiftamt;
  }

  // SDL
  else {
    shiftamt = offset << 3;
    data = rt >> shiftamt;
    dqm = mask >> shiftamt;
  }

  exdc_latch->request.vaddr = address & ~0x3ULL;
  exdc_latch->request.data = data;
  exdc_latch->request.wdqm = dqm;
  exdc_latch->request.access_type = VR4300_ACCESS_DWORD;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = 8;
  return 0;
}

//
// SWL
// SWR
//
int VR4300_SWL_SWR(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint64_t address = rs + (int64_t) (int16_t) iw;
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
  exdc_latch->request.wdqm = dqm;
  exdc_latch->request.access_type = VR4300_ACCESS_WORD;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = 4;
  return 0;
}

//
// SYSCALL
//
int VR4300_SYSCALL(struct vr4300 *vr4300,
  uint32_t unused(iw), uint64_t unused(rs), uint64_t unused(rt)) {
  VR4300_SYSC(vr4300);
  return 1;
}

// Function lookup table.
cen64_align(const vr4300_function
  vr4300_function_table[NUM_VR4300_OPCODES], CACHE_LINE_SIZE) = {
#define X(op) op,
#include "vr4300/opcodes.md"
#undef X
};

