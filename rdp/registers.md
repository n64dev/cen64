//
// rdp/registers.md: RDP register enumerations.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef DP_REGISTER_LIST
#define DP_REGISTER_LIST \
  X(DPC_START_REG) \
  X(DPC_END_REG) \
  X(DPC_CURRENT_REG) \
  X(DPC_STATUS_REG) \
  X(DPC_CLOCK_REG) \
  X(DPC_BUFBUSY_REG) \
  X(DPC_PIPEBUSY_REG) \
  X(DPC_TMEM_REG)
#endif

DP_REGISTER_LIST

