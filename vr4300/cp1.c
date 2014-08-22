//
// vr4300/cp1.c: VR4300 floating point unit coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "decoder.h"
#include "fpu/fpu.h"
#include "vr4300/cp1.h"
#include "vr4300/cpu.h"

static bool vr4300_cp1_usable(const struct vr4300 *vr4300);

//
// Invalid operation exception.
//
void VR4300_INV(struct vr4300 *vr4300);

//
// ABS.fmt
//
int VR4300_CP1_ABS(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t fd32;

    fpu_abs_32(&fs32, &fd32);
    result = fd32;
  }

  else
    fpu_abs_64(&fs, &result);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// ADD.fmt
//
int VR4300_CP1_ADD(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;
    uint32_t fd32;

    fpu_add_32(&fs32, &ft32, &fd32);
    result = fd32;
  }

  else
    fpu_add_64(&fs, &ft, &result);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// BC1F
// BC1FL
// BC1T
// BC1TL
//
int VR4300_BC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  unsigned opcode = (iw >> 16) & 0x3;

  uint64_t offset = (uint64_t) ((int16_t) iw) << 2;
  uint64_t taken_pc = rfex_latch->common.pc + (offset + 4);
  uint32_t cond = vr4300->regs[VR4300_CP1_FCR31];

  // XXX: The VR4300 manual says that the results of a FPU
  // FCR writes aren't ready on the next cycle, but it seems
  // that this might actually not be a limitiation of the real
  // hardware?
  if (vr4300->pipeline.dcwb_latch.dest == VR4300_CP1_FCR31)
    cond = vr4300->pipeline.dcwb_latch.result;

  switch (opcode) {
    case 0x0: // BC1F
      if (!(cond >> 23 & 0x1))
        icrf_latch->pc = taken_pc;
      break;

    case 0x1: // BC1T
      if (cond >> 23 & 0x1)
        icrf_latch->pc = taken_pc;
      break;

    case 0x2: // BC1FL
      if (!(cond >> 23 & 0x1))
        icrf_latch->pc = taken_pc;
      else
        rfex_latch->iw_mask = 0;
      break;

    case 0x3: // BC1TL
      if (cond >> 23 & 0x1)
        icrf_latch->pc = taken_pc;
      else
        rfex_latch->iw_mask = 0;
      break;
  }

  return 0;
}

//
// C.eq.fmt
// C.seq.fmt
//
int VR4300_CP1_C_EQ_C_SEQ(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_eq_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_eq_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// C.f.fmt
// C.sf.fmt
//
int VR4300_CP1_C_F_C_SF(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_f_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_f_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// C.ole.fmt
// C.le.fmt
//
int VR4300_CP1_C_OLE_C_LE(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_ole_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_ole_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// C.olt.fmt
// C.lt.fmt
//
int VR4300_CP1_C_OLT_C_LT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_olt_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_olt_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// C.ueq.fmt
// C.ngl.fmt
//
int VR4300_CP1_C_UEQ_C_NGL(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_ueq_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_ueq_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// C.ule.fmt
// C.ngt.fmt
//
int VR4300_CP1_C_ULE_C_NGT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_ule_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_ule_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// C.ult.fmt
// C.nge.fmt
//
int VR4300_CP1_C_ULT_C_NGE(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_ult_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_ult_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// C.un.fmt
// C.ngle.fmt
//
int VR4300_CP1_C_UN_C_NGLE(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = VR4300_CP1_FCR31;
  uint64_t result = vr4300->regs[dest];
  fpu_state_t saved_state;
  uint8_t flag;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  // Clear out C bit.
  result &= ~(1 << 23);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;

    flag = fpu_cmp_un_32(&fs32, &ft32);
  }

  else
    flag = fpu_cmp_un_64(&fs, &ft);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result | (flag << 23);
  exdc_latch->dest = dest;
  return 0;
}

//
// CEIL.l.fmt
//
int VR4300_CP1_CEIL_L(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_ceil_ss and _mm_ceil_sd.
  //
  fpu_set_state((vr4300->cp1.native_state &
    ~FPU_ROUND_MASK) | FPU_ROUND_POSINF);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i64_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i64_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_ceil_ss and _mm_ceil_sd.
  //
  vr4300->cp1.native_state =
    (vr4300->cp1.native_state & FPU_ROUND_MASK) |
    (fpu_get_state() & ~FPU_ROUND_MASK);

  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CEIL.w.fmt
//
int VR4300_CP1_CEIL_W(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint32_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_ceil_ss and _mm_ceil_sd.
  //
  fpu_set_state((vr4300->cp1.native_state &
    ~FPU_ROUND_MASK) | FPU_ROUND_POSINF);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i32_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i32_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_ceil_ss and _mm_ceil_sd.
  //
  vr4300->cp1.native_state =
    (vr4300->cp1.native_state & FPU_ROUND_MASK) |
    (fpu_get_state() & ~FPU_ROUND_MASK);

  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CFC1
//
int VR4300_CFC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  unsigned dest = GET_RT(iw);
  unsigned src = GET_RD(iw);
  uint64_t result;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  switch (src) {
    case 0: src = VR4300_CP1_FCR0; break;
    case 31: src = VR4300_CP1_FCR31; break;

    default:
      src = 0;

      assert(0 && "CFC1: Read reserved FCR.");
      break;
  }

  result = vr4300->regs[src];

  // XXX: The VR4300 manual says that the results of a FPU
  // FCR writes aren't ready on the next cycle, but it seems
  // that this might actually not be a limitiation of the real
  // hardware?
  if (vr4300->pipeline.dcwb_latch.dest == VR4300_CP1_FCR31)
    result = vr4300->pipeline.dcwb_latch.result;

  // Undefined while the next instruction
  // executes, so we can cheat and use the RF.
  exdc_latch->result = (int32_t) result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CTC1
//
// XXX: Raise exception on cause/enable.
// XXX: In such cases, ensure write occurs.
//
int VR4300_CTC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RD(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  if (dest == 31)
    dest = VR4300_CP1_FCR31;

  else {
    assert(0 && "CTC1: Write to fixed/reserved FCR.");

    dest = 0;
    rt = 0;
  }

  // Undefined while the next instruction
  // executes, so we can cheat and use WB.
  exdc_latch->result = rt;
  exdc_latch->dest = dest;
  return 0;
}

//
// CVT.d.fmt
//
int VR4300_CP1_CVT_D(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt == VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_f64_f32(&fs32, &result); break;
    case VR4300_FMT_W: fpu_cvt_f64_i32(&fs32, &result); break;
    case VR4300_FMT_L: fpu_cvt_f64_i64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CVT.l.fmt
//
int VR4300_CP1_CVT_L(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i64_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i64_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CVT.s.fmt
//
int VR4300_CP1_CVT_S(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint32_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt == VR4300_FMT_S) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  switch (fmt) {
    case VR4300_FMT_D: fpu_cvt_f32_f64(&fs, &result); break;
    case VR4300_FMT_W: fpu_cvt_f32_i32(&fs32, &result); break;
    case VR4300_FMT_L: fpu_cvt_f32_i64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CVT.w.fmt
//
int VR4300_CP1_CVT_W(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint32_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i32_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i32_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// DIV.fmt
//
int VR4300_CP1_DIV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;
    uint32_t fd32;

    fpu_div_32(&fs32, &ft32, &fd32);
    result = fd32;
  }

  else
    fpu_div_64(&fs, &ft, &result);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// FLOOR.l.fmt
//
int VR4300_CP1_FLOOR_L(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_floor_ss and _mm_floor_sd.
  //
  fpu_set_state((vr4300->cp1.native_state &
    ~FPU_ROUND_MASK) | FPU_ROUND_NEGINF);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i64_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i64_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_floor_ss and _mm_floor_sd.
  //
  vr4300->cp1.native_state =
    (vr4300->cp1.native_state & FPU_ROUND_MASK) |
    (fpu_get_state() & ~FPU_ROUND_MASK);

  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// FLOOR.w.fmt
//
int VR4300_CP1_FLOOR_W(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint32_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  //
  // TODO: (Optionally) compile with
  // _mm_floor_ss and _mm_floor_sd.
  //
  fpu_set_state((vr4300->cp1.native_state &
    ~FPU_ROUND_MASK) | FPU_ROUND_NEGINF);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i32_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i32_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_floor_ss and _mm_floor_sd.
  //
  vr4300->cp1.native_state =
    (vr4300->cp1.native_state & FPU_ROUND_MASK) |
    (fpu_get_state() & ~FPU_ROUND_MASK);

  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// LDC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LDC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_FT(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

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
// LWC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LWC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint64_t address = (rs + (int16_t) iw);
  unsigned dest = GET_FT(iw);

  uint64_t result = 0;
  unsigned postshift = 0;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  if (!(status & 0x04000000)) {
    result = dest & 0x1
      ? ft & 0x00000000FFFFFFFFULL
      : ft & 0xFFFFFFFF00000000ULL;

    postshift = (dest & 0x1) << 5;
    dest &= ~0x1;
  }

  exdc_latch->request.vaddr = address;
  exdc_latch->request.dqm = ~0U;
  exdc_latch->request.postshift = postshift;
  exdc_latch->request.two_words = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = 4;

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// MUL.fmt
//
int VR4300_CP1_MUL(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;
    uint32_t fd32;

    fpu_mul_32(&fs32, &ft32, &fd32);
    result = fd32;
  }

  else
    fpu_mul_64(&fs, &ft, &result);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// MFC1
//
int VR4300_MFC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t unused(rt)) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_RT(iw);
  uint64_t result;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  // TODO/FIXME: XXX
  assert(!(GET_FS(iw) & 0x1));
  result = (int32_t) fs;

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// MOV.fmt
//
int VR4300_CP1_MOV(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  unsigned dest = GET_FD(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  exdc_latch->result = fs;
  exdc_latch->dest = dest;
  return 0;
}

//
// MTC1
//
int VR4300_MTC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t rt) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  uint64_t result = (int32_t) rt;
  unsigned dest = GET_FS(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  if (!(status & 0x04000000)) {
    result = (dest & 0x1)
      ? ((uint32_t) fs) | (rt << 32)
      : (fs & ~0xFFFFFFFFULL) | ((uint32_t) rt);

    dest &= ~0x1;
  }

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// NEG.fmt
//
int VR4300_CP1_NEG(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t fd32;

    fpu_neg_32(&fs32, &fd32);
    result = fd32;
  }

  else
    fpu_neg_64(&fs, &result);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// ROUND.l.fmt
//
int VR4300_CP1_ROUND_L(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_round_ss and _mm_round_sd.
  //
  fpu_set_state((vr4300->cp1.native_state &
    ~FPU_ROUND_MASK) | FPU_ROUND_NEAREST);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i64_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i64_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_round_ss and _mm_round_sd.
  //
  vr4300->cp1.native_state =
    (vr4300->cp1.native_state & FPU_ROUND_MASK) |
    (fpu_get_state() & ~FPU_ROUND_MASK);

  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// ROUND.w.fmt
//
int VR4300_CP1_ROUND_W(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint32_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_round_ss and _mm_round_sd.
  //
  fpu_set_state((vr4300->cp1.native_state &
    ~FPU_ROUND_MASK) | FPU_ROUND_NEAREST);

  switch (fmt) {
    case VR4300_FMT_S: fpu_cvt_i32_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_cvt_i32_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  //
  // TODO: (Optionally) compile with
  // _mm_round_ss and _mm_round_sd.
  //
  vr4300->cp1.native_state =
    (vr4300->cp1.native_state & FPU_ROUND_MASK) |
    (fpu_get_state() & ~FPU_ROUND_MASK);

  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// SDC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_SDC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  exdc_latch->request.vaddr = rs + (int16_t) iw;
  exdc_latch->request.data = ft;
  exdc_latch->request.dqm = ~0ULL;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = 8;

  return 0;
}

//
// SQRT.fmt
//
int VR4300_CP1_SQRT(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t fd32;

    fpu_sqrt_32(&fs32, &fd32);
    result = fd32;
  }

  else
    fpu_sqrt_64(&fs, &result);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// SUB.fmt
//
int VR4300_CP1_SUB(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  if (fmt == VR4300_FMT_S) {
    uint32_t fs32 = fs;
    uint32_t ft32 = ft;
    uint32_t fd32;

    fpu_sub_32(&fs32, &ft32, &fd32);
    result = fd32;
  }

  else
    fpu_sub_64(&fs, &ft, &result);

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// SWC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_SWC1(struct vr4300 *vr4300,
  uint32_t iw, uint64_t rs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  unsigned ft_reg = GET_FT(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  if (!(status & 0x04000000))
    ft >>= ((ft_reg & 0x1) << 5);

  exdc_latch->request.vaddr = rs + (int16_t) iw;
  exdc_latch->request.data = ft;
  exdc_latch->request.dqm = ~0U;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = 4;

  return 0;
}

//
// TRUNC.l.fmt
//
int VR4300_CP1_TRUNC_L(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint64_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  switch (fmt) {
    case VR4300_FMT_S: fpu_trunc_i64_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_trunc_i64_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// TRUNC.w.fmt
//
int VR4300_CP1_TRUNC_W(struct vr4300 *vr4300,
  uint32_t iw, uint64_t fs, uint64_t ft) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  enum vr4300_fmt fmt = GET_FMT(iw);
  unsigned dest = GET_FD(iw);
  fpu_state_t saved_state;

  uint32_t fs32 = fs;
  uint32_t result;

  saved_state = fpu_get_state();

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  else if (fmt != VR4300_FMT_S && fmt != VR4300_FMT_D) {
    VR4300_INV(vr4300);
    return 1;
  }

  fpu_set_state(vr4300->cp1.native_state);

  switch (fmt) {
    case VR4300_FMT_S: fpu_trunc_i32_f32(&fs32, &result); break;
    case VR4300_FMT_D: fpu_trunc_i32_f64(&fs, &result); break;

    default:
      assert(0 && "Unknown case?");
      result = 0;
      break;
  }

  vr4300->cp1.native_state = fpu_get_state();
  fpu_set_state(saved_state);

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

// Initializes the coprocessor.
void vr4300_cp1_init(struct vr4300 *vr4300) {
  vr4300->cp1.native_state = FPU_ROUND_NEAREST | FPU_MASK_EXCPS;
}

// Determines if the coprocessor was used yet.
bool vr4300_cp1_usable(const struct vr4300 *vr4300) {
  return (vr4300->regs[VR4300_CP0_REGISTER_STATUS] & 0x20000000) != 0;
}

