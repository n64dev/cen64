//
// rsp/interface.c: RSP interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "rsp/cpu.h"
#include "rsp/interface.h"

// Reads a word from the SP memory MMIO register space.
int read_sp_mem(void *opaque, uint32_t address, uint32_t *word) {
  return 0;
}

// Writes a word to the SP memory MMIO register space.
int write_sp_mem(void *opaque, uint32_t address, uint32_t *word) {
  return 0;
}

// Reads a word from the SP MMIO register space.
int read_sp_regs(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - SP_REGS_BASE_ADDRESS;
  enum sp_register reg = SP_MEM_ADDR_REG + (offset >> 2);
  struct rsp *rsp = (struct rsp *) opaque;

  *word = rsp->regs[reg];
  return 0;
}

// Writes a word to the SP MMIO register space.
int write_sp_regs(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - 0x04040000;
  enum sp_register reg = SP_MEM_ADDR_REG + (offset >> 2);
  struct rsp *rsp = (struct rsp *) opaque;

  return 0;
}

// Reads a word from the (high) SP MMIO register space.
int read_sp_regs2(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - SP_REGS2_BASE_ADDRESS;
  struct rsp *rsp = (struct rsp *) opaque;

  abort();
  return 0;
}

// Writes a word to the (high) SP MMIO register space.
int write_sp_regs2(void *opaque, uint32_t address, uint32_t *word) {
  uint32_t offset = address - 0x04040000;
  enum sp_register reg = SP_MEM_ADDR_REG + (offset >> 2);
  struct rsp *rsp = (struct rsp *) opaque;

  return 0;
}

