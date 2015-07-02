//
// vr4300/fault.md: VR4300 fault management.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef VR4300_FAULT_LIST
#define VR4300_FAULT_LIST \
  X(NONE) X(CP0I) X(RST) X(NMI) X(OVFL) X(TRAP) X(FPE) X(DADE) \
  X(DTLB) X(WAT) X(INTR) X(DCM) X(DCB) X(COP) X(DBE) X(SYSC) X(BRPT) \
  X(CPU) X(RSVD) X(LDI) X(MCI) X(IADE) X(ITM) X(ICB) X(UNC) X(ITLB) \
  X(IBE)
#endif

VR4300_FAULT_LIST

