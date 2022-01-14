//
// rsp/interface.h: RSP interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __rsp_interface_h__
#define __rsp_interface_h__
#include "common.h"

void rsp_dma_read(struct rsp *rsp);
void rsp_dma_write(struct rsp *rsp);

int read_sp_mem(void *opaque, uint32_t address, uint32_t *word);
int read_sp_regs(void *opaque, uint32_t address, uint32_t *word);
int read_sp_regs2(void *opaque, uint32_t address, uint32_t *word);
int write_sp_mem(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int write_sp_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);
int write_sp_regs2(void *opaque, uint32_t address, uint32_t word, uint32_t dqm);

#endif

