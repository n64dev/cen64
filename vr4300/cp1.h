//
// vr4300/cp1.h: VR4300 floating point unit coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_cp1_h__
#define __vr4300_cp1_h__
#include "common.h"
#include "fpu.h"

struct vr4300;

struct vr4300_cp1 {
  fpu_state_t native_state;
  fpu_state_t native_enables;
};

int VR4300_BC1(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CFC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt);
int VR4300_CTC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt);
int VR4300_LDC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt);
int VR4300_LWC1(struct vr4300 *vr4300, uint64_t rs, uint64_t ft);
int VR4300_MFC1(struct vr4300 *vr4300, uint64_t fs, uint64_t unused(rt));
int VR4300_MTC1(struct vr4300 *vr4300, uint64_t unused(rs), uint64_t rt);
int VR4300_SDC1(struct vr4300 *vr4300, uint64_t rs, uint64_t ft);
int VR4300_SWC1(struct vr4300 *vr4300, uint64_t rs, uint64_t ft);

int VR4300_CP1_ABS(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_ADD(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_EQ(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_F(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_LE(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_LT(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_NGE(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_NGL(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_NGLE(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_NGT(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_OLE(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_OLT(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_SEQ(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_SF(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_UEQ(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_ULE(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_ULT(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_C_UN(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_CEIL_L(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_CEIL_W(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_CVT_D(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_CVT_L(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_CVT_S(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_CVT_W(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_DIV(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_FLOOR_L(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_FLOOR_W(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_MOV(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_MUL(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_NEG(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_ROUND_L(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_ROUND_W(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_SQRT(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_SUB(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_TRUNC_L(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);
int VR4300_CP1_TRUNC_W(struct vr4300 *vr4300, uint64_t fs, uint64_t ft);

void vr4300_cp1_init(struct vr4300 *vr4300);

#endif

