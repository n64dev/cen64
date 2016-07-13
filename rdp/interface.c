//
// rdp/interface.c: RDP interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "rdp/cpu.h"
#include "rdp/interface.h"
#include "thread.h"

#define DP_XBUS_DMEM_DMA          0x00000001
#define DP_FREEZE                 0x00000002
#define DP_FLUSH                  0x00000004

#define DP_CLEAR_XBUS_DMEM_DMA    0x00000001
#define DP_SET_XBUS_DMEM_DMA      0x00000002
#define DP_CLEAR_FREEZE           0x00000004
#define DP_SET_FREEZE             0x00000008
#define DP_CLEAR_FLUSH            0x00000010
#define DP_SET_FLUSH              0x00000020

int rdp_process_list(struct rdp *rdp);

// Reads a word from the DP MMIO register space.
int read_dp_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct rdp *rdp = (struct rdp *) opaque;
  uint32_t offset = address - DP_REGS_BASE_ADDRESS;
  enum dp_register reg = (offset >> 2);

  *word = rdp->regs[reg];
  debug_mmio_read(dp, dp_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the DP MMIO register space.
int write_dp_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rdp *rdp = (struct rdp *) opaque;
  uint32_t offset = address - DP_REGS_BASE_ADDRESS;
  enum dp_register reg = (offset >> 2);
  int gordp;

  debug_mmio_write(dp, dp_register_mnemonics[reg], word, dqm);

  cen64_mutex_lock(&rdp->rdp_mutex);

  switch (reg) {
    case DPC_START_REG:
      if (!(rdp->regs[DPC_STATUS_REG] & 0x400)) {
        rdp->regs[DPC_CURRENT_REG] = word;
        rdp->regs[DPC_START_REG] = word;
      }

      break;

    case DPC_END_REG:
      rdp->regs[DPC_END_REG] = word;
      gordp = rdp_process_list(rdp);
      break;

    case DPC_STATUS_REG:
      if (word & DP_CLEAR_XBUS_DMEM_DMA)
        rdp->regs[DPC_STATUS_REG] &= ~DP_XBUS_DMEM_DMA;
      else if (word & DP_SET_XBUS_DMEM_DMA)
        rdp->regs[DPC_STATUS_REG] |= DP_XBUS_DMEM_DMA;

      if (word & DP_CLEAR_FREEZE)
        rdp->regs[DPC_STATUS_REG] &= ~DP_FREEZE;
//      else if (word & DP_SET_FREEZE)
//        rdp->regs[DPC_STATUS_REG] |= DP_FREEZE;

      if (word & DP_CLEAR_FLUSH)
        rdp->regs[DPC_STATUS_REG] &= ~DP_FLUSH;
      else if (word & DP_SET_FLUSH)
        rdp->regs[DPC_STATUS_REG] |= DP_FLUSH;
      break;

    default:
      rdp->regs[reg] &= ~dqm;
      rdp->regs[reg] |= word;
      break;
  }

  cen64_mutex_unlock(&rdp->rdp_mutex);

  if (reg == DPC_END_REG && !gordp)
    cen64_cv_signal(&rdp->rdp_signal);

  return 0;
}

