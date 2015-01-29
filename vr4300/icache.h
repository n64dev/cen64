//
// vr4300/icache.h: VR4300 instruction cache.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_icache_h__
#define __vr4300_icache_h__
#include "common.h"

struct vr4300_icache_line {
  uint8_t data[8 * 4];
  uint32_t metadata;
};

struct vr4300_icache {
  struct vr4300_icache_line lines[512];
};

cen64_cold void vr4300_icache_init(struct vr4300_icache *icache);

cen64_hot const struct vr4300_icache_line* vr4300_icache_probe(
  const struct vr4300_icache *icache, uint64_t vaddr, uint32_t paddr);

void vr4300_icache_fill(struct vr4300_icache *icache,
  uint64_t vaddr, uint32_t paddr, const void *data);
uint32_t vr4300_icache_get_tag(const struct vr4300_icache *icache,
  uint64_t vaddr);
void vr4300_icache_invalidate(struct vr4300_icache *icache, uint64_t vaddr);
void vr4300_icache_invalidate_hit(struct vr4300_icache *icache,
  uint64_t vaddr, uint32_t paddr);
void vr4300_icache_set_taglo(struct vr4300_icache *icache,
  uint64_t vaddr, uint32_t tag);

#endif

