//
// rsp/interface.c: RSP interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "rsp/cp0.h"
#include "rsp/cpu.h"
#include "rsp/interface.h"

// DMA into the RSP's memory space.
void rsp_dma_read(struct rsp *rsp) {
  uint32_t length = (rsp->regs[RSP_CP0_REGISTER_DMA_READ_LENGTH] & 0xFFF) + 1;
  uint32_t skip = rsp->regs[RSP_CP0_REGISTER_DMA_READ_LENGTH] >> 20 & 0xFFF;
  unsigned count = rsp->regs[RSP_CP0_REGISTER_DMA_READ_LENGTH] >> 12 & 0xFF;
  unsigned j, i = 0;

  // Force alignment.
  length = (length + 0x7) & ~0x7;
  rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] &= ~0x3;
  rsp->regs[RSP_CP0_REGISTER_DMA_DRAM] &= ~0x7;

  // Check length.
  if (((rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] & 0xFFF) + length) > 0x1000)
    length = 0x1000 - (rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] & 0xFFF);

  do {
    uint32_t source = rsp->regs[RSP_CP0_REGISTER_DMA_DRAM] & 0x7FFFFC;
    uint32_t dest = rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] & 0x1FFC;
    j = 0;

    do {
      uint32_t source_addr = (source + j) & 0x7FFFFC;
      uint32_t dest_addr = (dest + j) & 0x1FFC;
      uint32_t word;

      bus_read_word(rsp->bus, source_addr, &word);

      // Update opcode cache.
      if (dest_addr & 0x1000) {
        rsp->opcode_cache[(dest_addr - 0x1000) >> 2] =
          *rsp_decode_instruction(word);
      } else {
        word = byteswap_32(word);
      }

      memcpy(rsp->mem + dest_addr, &word, sizeof(word));
      j += 4;
    } while (j < length);

    rsp->regs[RSP_CP0_REGISTER_DMA_DRAM] += length + skip;
    rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] += length;
  } while(++i <= count);
}

// DMA from the RSP's memory space.
void rsp_dma_write(struct rsp *rsp) {
  uint32_t length = (rsp->regs[RSP_CP0_REGISTER_DMA_WRITE_LENGTH] & 0xFFF) + 1;
  uint32_t skip = rsp->regs[RSP_CP0_REGISTER_DMA_WRITE_LENGTH] >> 20 & 0xFFF;
  unsigned count = rsp->regs[RSP_CP0_REGISTER_DMA_WRITE_LENGTH] >> 12 & 0xFF;
  unsigned j, i = 0;

  // Force alignment.
  length = (length + 0x7) & ~0x7;
  rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] &= ~0x3;
  rsp->regs[RSP_CP0_REGISTER_DMA_DRAM] &= ~0x7;

  // Check length.
  if (((rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] & 0xFFF) + length) > 0x1000)
    length = 0x1000 - (rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] & 0xFFF);

  do {
    uint32_t dest = rsp->regs[RSP_CP0_REGISTER_DMA_DRAM] & 0x7FFFFC;
    uint32_t source = rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] & 0x1FFC;
    j = 0;

    do {
      uint32_t source_addr = (source + j) & 0x1FFC;
      uint32_t dest_addr = (dest + j) & 0x7FFFFC;
      uint32_t word;

      memcpy(&word, rsp->mem + source_addr, sizeof(word));

      if (!(source_addr & 0x1000))
        word = byteswap_32(word);

      bus_write_word(rsp->bus, dest_addr, word, ~0U);
      j += 4;
    } while (j < length);

    rsp->regs[RSP_CP0_REGISTER_DMA_CACHE] += length;
    rsp->regs[RSP_CP0_REGISTER_DMA_DRAM] += length + skip;
  } while (++i <= count);
}

// Reads a word from the SP memory MMIO register space.
int read_sp_mem(void *opaque, uint32_t address, uint32_t *word) {
  struct rsp *rsp = (struct rsp *) opaque;
  unsigned offset = address & 0x1FFC;

  memcpy(word, rsp->mem + offset, sizeof(*word));

  if (!(offset & 0x1000))
    *word = byteswap_32(*word);

  return 0;
}

// Reads a word from the SP MMIO register space.
int read_sp_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2);

  *word = rsp_read_cp0_reg(rsp, reg);
  debug_mmio_read(sp, sp_register_mnemonics[reg], *word);
  return 0;
}

// Reads a word from the (high) SP MMIO register space.
int read_sp_regs2(void *opaque, uint32_t address, uint32_t *word) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS2_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2) + SP_PC_REG;

  if (reg == SP_PC_REG)
    *word = rsp->pipeline.dfwb_latch.common.pc;

  else
    abort();

  debug_mmio_read(sp, sp_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the SP memory MMIO register space.
int write_sp_mem(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rsp *rsp = (struct rsp *) opaque;
  unsigned offset = address & 0x1FFC;
  uint32_t orig_word;

  memcpy(&orig_word, rsp->mem + offset, sizeof(orig_word));
  orig_word = byteswap_32(orig_word) & ~dqm;
  word = orig_word | word;

  // Update opcode cache.
  if (offset & 0x1000) {
    rsp->opcode_cache[(offset - 0x1000) >> 2] = *rsp_decode_instruction(word);
  } else {
    word = byteswap_32(word);
  }

  memcpy(rsp->mem + offset, &word, sizeof(word));
  return 0;
}

// Writes a word to the SP MMIO register space.
int write_sp_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2);

  debug_mmio_write(sp, sp_register_mnemonics[reg], word, dqm);
  rsp_write_cp0_reg(rsp, reg, word);
  return 0;
}

// Writes a word to the (high) SP MMIO register space.
int write_sp_regs2(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct rsp *rsp = (struct rsp *) opaque;
  uint32_t offset = address - SP_REGS2_BASE_ADDRESS;
  enum sp_register reg = (offset >> 2) + SP_PC_REG;

  debug_mmio_write(sp, sp_register_mnemonics[reg], word, dqm);

  if (reg == SP_PC_REG)
    rsp->pipeline.ifrd_latch.pc = word & 0xFFC;

  else
    abort();

  return 0;
}

