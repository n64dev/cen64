//
// rsp/vector_opcodes.md: RSP vector opcode types and info.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSP_VECTOR_OPCODE_TABLE
#define RSP_VECTOR_OPCODE_TABLE X(VINVALID) \
  X(VABS) X(VADD) X(VADDC) X(VAND) X(VCH) X(VCL) X(VCR) X(VEQ) X(VGE) \
  X(VLT) X(VMACF) X(VMACQ) X(VMACU) X(VMADH) X(VMADL) X(VMADM) X(VMADN) \
  X(VMOV) X(VMRG) X(VMUDH) X(VMUDL) X(VMUDM) X(VMUDN) X(VMULF) X(VMULQ) \
  X(VMULU) X(VNAND) X(VNE) X(VNOP) X(VNOR) X(VNULL) X(VNXOR) X(VOR) X(VRCP) \
  X(VRCPH) X(VRCPL) X(VRNDN) X(VRNDP) X(VRSQ) X(VRSQH) X(VRSQL) X(VSAR) \
  X(VSUB) X(VSUBC) X(VXOR) 
#endif

RSP_VECTOR_OPCODE_TABLE

