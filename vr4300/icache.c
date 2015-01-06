//
// vr4300/icache.c: VR4300 instruction cache.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/icache.h"

static inline struct vr4300_icache_line* get_line(
  struct vr4300_icache *icache, uint64_t vaddr);
static inline const struct vr4300_icache_line* get_line_const(
  const struct vr4300_icache *icache, uint64_t vaddr);

static inline uint32_t get_tag(const struct vr4300_icache_line *line);
static void invalidate_line(struct vr4300_icache_line *line);
static bool is_valid(const struct vr4300_icache_line *line);
static void set_taglo(struct vr4300_icache_line *line, uint32_t taglo);
static void validate_line(struct vr4300_icache_line *line, uint32_t tag);

// Returns the line for a given virtual address.
struct vr4300_icache_line* get_line(
  struct vr4300_icache *icache, uint64_t vaddr) {
  return icache->lines + (vaddr >> 5 & 0x1FF);
}

// Returns the line for a given virtual address.
const struct vr4300_icache_line* get_line_const(
  const struct vr4300_icache *icache, uint64_t vaddr) {
  return icache->lines + (vaddr >> 5 & 0x1FF);
}

// Returns the physical tag associated with the line.
uint32_t get_tag(const struct vr4300_icache_line *line) {
  return line->metadata >> 12;
}

// Invalidates the line, but leaves the physical tag untouched.
void invalidate_line(struct vr4300_icache_line *line) {
  line->metadata &= ~0x1;
}

// Returns true if the line is valid, otherwise returns false.
bool is_valid(const struct vr4300_icache_line *line) {
  return (line->metadata & 0x1) == 0x1;
}

// Sets the tag of the specified line and valid bit.
void set_taglo(struct vr4300_icache_line *line, uint32_t taglo) {
  line->metadata = (taglo << 4 & 0xFFFFF000) | (taglo >> 7 & 0x1);
}

// Sets the line's physical tag and validates the line.
static void validate_line(struct vr4300_icache_line *line, uint32_t tag) {
  line->metadata = (tag << 12) | 0x1;
}

// Fills an instruction cache line with data.
void vr4300_icache_fill(struct vr4300_icache *icache,
  uint64_t vaddr, uint32_t paddr, const void *data) {
  struct vr4300_icache_line *line = get_line(icache, vaddr);

  memcpy(line->data, data, sizeof(line->data));
  validate_line(line, paddr >> 5);
}

// Returns the tag of the line associated with vaddr.
uint32_t vr4300_icache_get_tag(const struct vr4300_icache *icache,
  uint64_t vaddr) {
  const struct vr4300_icache_line *line = get_line_const(icache, vaddr);

  return get_tag(line);
}

// Initializes the instruction cache.
void vr4300_icache_init(struct vr4300_icache *icache) {
}

// Invalidates an instruction cache line (regardless if hit or miss).
void vr4300_icache_invalidate(struct vr4300_icache *icache, uint64_t vaddr) {
  struct vr4300_icache_line *line = get_line(icache, vaddr);

  invalidate_line(line);
}

// Invalidates an instruction cache line (only on a hit).
void vr4300_icache_invalidate_hit(struct vr4300_icache *icache,
  uint64_t vaddr, uint32_t paddr) {
  struct vr4300_icache_line *line = get_line(icache, vaddr);
  uint32_t ptag = get_tag(line);

  if (ptag == (paddr >> 5) && is_valid(line))
    invalidate_line(line);
}

// Probes the instruction cache for a matching line.
const struct vr4300_icache_line* vr4300_icache_probe(
  const struct vr4300_icache *icache, uint64_t vaddr, uint32_t paddr) {
  const struct vr4300_icache_line *line = get_line_const(icache, vaddr);
  uint32_t ptag = get_tag(line);

  // Virtually index, and physically tagged.
  if (ptag == (paddr >> 5) && is_valid(line))
    return line;

  return NULL;
}

// Sets the physical tag associated with the line.
void vr4300_icache_set_taglo(struct vr4300_icache *icache,
  uint64_t vaddr, uint32_t taglo) {
  struct vr4300_icache_line *line = get_line(icache, vaddr);

  set_taglo(line, taglo);
}

