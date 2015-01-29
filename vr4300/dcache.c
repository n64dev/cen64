//
// vr4300/dcache.c: VR4300 instruction cache.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/dcache.h"

static inline struct vr4300_dcache_line* get_line(
  struct vr4300_dcache *dcache, uint64_t vaddr);

static inline uint32_t get_tag(const struct vr4300_dcache_line *line);
static inline bool is_valid(const struct vr4300_dcache_line *line);

static void invalidate_line(struct vr4300_dcache_line *line);
static bool is_dirty(const struct vr4300_dcache_line *line);
static void set_dirty(struct vr4300_dcache_line *line);
static void set_tag(struct vr4300_dcache_line *line, uint32_t tag);
static void set_taglo(struct vr4300_dcache_line *line, uint32_t taglo);
static void validate_line(struct vr4300_dcache_line *line, uint32_t tag);

// Returns the line for a given virtual address.
struct vr4300_dcache_line* get_line(
  struct vr4300_dcache *dcache, uint64_t vaddr) {
  return dcache->lines + (vaddr >> 4 & 0x1FF);
}

// Returns the physical tag associated with the line.
uint32_t get_tag(const struct vr4300_dcache_line *line) {
  return line->metadata & ~0xF;
}

// Invalidates the line, but leaves the physical tag untouched.
void invalidate_line(struct vr4300_dcache_line *line) {
  line->metadata &= ~0x1;
}

// Returns true if the line is valid, otherwise returns false.
bool is_dirty(const struct vr4300_dcache_line *line) {
  return (line->metadata & 0x2) == 0x2;
}

// Returns true if the line is valid, otherwise returns false.
bool is_valid(const struct vr4300_dcache_line *line) {
  return (line->metadata & 0x1) == 0x1;
}

// Sets the state of the line to dirty.
void set_dirty(struct vr4300_dcache_line *line) {
  line->metadata |= 0x2;
}

// Sets the tag of the specified line, retaining current valid bit.
void set_tag(struct vr4300_dcache_line *line, uint32_t tag) {
  line->metadata = tag | (line->metadata & 0x1);
}

// Sets the tag of the specified line and valid bit.
void set_taglo(struct vr4300_dcache_line *line, uint32_t taglo) {
  line->metadata = (taglo << 4 & 0xFFFFF000) | (taglo >> 7 & 0x1);
}

// Sets the line's physical tag and validates the line.
static void validate_line(struct vr4300_dcache_line *line, uint32_t tag) {
  line->metadata = tag | 0x1;
}

// Sets the physical tag associated with the line, marks as dirty.
void vr4300_dcache_create_dirty_exclusive(
  struct vr4300_dcache *dcache, uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);

  set_tag(line, paddr & ~0xF);
  set_dirty(line);
}

// Fills an instruction cache line with data.
void vr4300_dcache_fill(struct vr4300_dcache *dcache,
  uint64_t vaddr, uint32_t paddr, const void *data) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);

  memcpy(line->data, data, sizeof(line->data));
  validate_line(line, paddr & ~0xF);
}

// Returns the tag of the line associated with vaddr.
uint32_t vr4300_dcache_get_tag(const struct vr4300_dcache_line *line) {
  return get_tag(line);
}

// Gets the physical tag associated with the line.
uint32_t vr4300_dcache_get_taglo(struct vr4300_dcache *dcache, uint64_t vaddr) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);

  uint32_t taglo = is_valid(line) ? 0xC0 : 0x00;
  return taglo | (line->metadata >> 4 & 0x0FFFFF00U);
}

// Initializes the instruction cache.
void vr4300_dcache_init(struct vr4300_dcache *dcache) {
}

// Invalidates an instruction cache line (regardless if hit or miss).
void vr4300_dcache_invalidate(struct vr4300_dcache_line *line) {
  invalidate_line(line);
}

// Invalidates an instruction cache line (only on a hit).
void vr4300_dcache_invalidate_hit(struct vr4300_dcache *dcache,
  uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);
  uint32_t ptag = get_tag(line);

  if (ptag == (paddr & ~0xF) && is_valid(line))
    invalidate_line(line);
}

// Probes the instruction cache for a matching line.
struct vr4300_dcache_line* vr4300_dcache_probe(
  struct vr4300_dcache *dcache, uint64_t vaddr, uint32_t paddr) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);
  uint32_t ptag = get_tag(line);

  // Virtually index, and physically tagged.
  if (ptag == (paddr & ~0xF) && is_valid(line))
    return line;

  return NULL;
}

// Marks the line as dirty.
void vr4300_dcache_set_dirty(struct vr4300_dcache_line *line) {
  set_dirty(line);
}

// Sets the physical tag associated with the line.
void vr4300_dcache_set_taglo(struct vr4300_dcache *dcache,
  uint64_t vaddr, uint32_t taglo) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);

  set_taglo(line, taglo);
}

// Returns the line if it's dirty and valid.
// Call before replacement of writeback entry.
struct vr4300_dcache_line *vr4300_dcache_should_flush_line(
  struct vr4300_dcache *dcache, uint64_t vaddr) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);

  return is_dirty(line) && is_valid(line)
    ? line : NULL;
}

// Writes back the block if the line is valid, then invalidates the line.
struct vr4300_dcache_line *vr4300_dcache_wb_invalidate(
  struct vr4300_dcache *dcache, uint64_t vaddr) {
  struct vr4300_dcache_line *line = get_line(dcache, vaddr);

  if (is_valid(line)) {
    invalidate_line(line);
    return line;
  }

  return NULL;
}

