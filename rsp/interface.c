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
  struct rsp *rsp = (struct rsp *) opaque;
  unsigned offset = address & 0x1FFC;

  memcpy(word, rsp->mem + offset, sizeof(*word));
  *word = byteswap_32(*word);
  return 0;
}

// Reads a word from the SP MMIO register space.
int read_sp_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2);

  *word = rsp->regs[reg + SP_REGISTER_OFFSET];
  debug_mmio_read(rsp, sp_register_mnemonics[reg], *word);
  return 0;
}

// Reads a word from the (high) SP MMIO register space.
int read_sp_regs2(void *opaque, uint32_t address, uint32_t *word) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS2_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2) + SP_PC_REG;

  *word = rsp->regs[reg + SP_REGISTER_OFFSET];
  debug_mmio_read(rsp, sp_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the SP memory MMIO register space.
int write_sp_mem(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rsp *rsp = (struct rsp *) opaque;
  unsigned offset = address & 0x1FFC;
  uint32_t orig_word;

  memcpy(&orig_word, rsp->mem + offset, sizeof(orig_word));
  orig_word = byteswap_32(orig_word) & ~dqm;
  word = byteswap_32(orig_word | word);
  memcpy(rsp->mem + offset, &word, sizeof(word));
  return 0;
}

// Writes a word to the SP MMIO register space.
int write_sp_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2);

  debug_mmio_write(rsp, sp_register_mnemonics[reg], word, dqm);
  rsp->regs[reg + SP_REGISTER_OFFSET] &= ~dqm;
  rsp->regs[reg + SP_REGISTER_OFFSET] |= word;
  return 0;
}

// Writes a word to the (high) SP MMIO register space.
int write_sp_regs2(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS2_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2) + SP_PC_REG;

  debug_mmio_write(rsp, sp_register_mnemonics[reg], word, dqm);
  rsp->regs[reg + SP_REGISTER_OFFSET] &= ~dqm;
  rsp->regs[reg + SP_REGISTER_OFFSET] |= word;
  return 0;
}

