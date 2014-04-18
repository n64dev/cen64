//
// bus/memory_map.h: System memory mapper.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __bus_memory_map_h__
#define __bus_memory_map_h__
#include "common.h"

// Callback functions to handle reads/writes.
typedef int (*memory_rd_function)(void *, uint32_t, uint32_t *);
typedef int (*memory_wr_function)(void *, uint32_t, uint32_t, uint32_t);

enum memory_map_color {
  MEMORY_MAP_BLACK,
  MEMORY_MAP_RED
};

struct memory_mapping {
  void *instance;

  memory_rd_function on_read;
  memory_wr_function on_write;

  uint32_t length;
  uint32_t start;
  uint32_t end;
};

struct memory_map_node {
  struct memory_map_node *left;
  struct memory_map_node *parent;
  struct memory_map_node *right;

	struct memory_mapping mapping;
  enum memory_map_color color;
};

struct memory_map {
  struct memory_map_node *mappings;
  struct memory_map_node *nil;
  struct memory_map_node *root;

  unsigned next_map_index;
  unsigned num_mappings;
};

struct memory_map* create_memory_map(unsigned num_mappings);
void destroy_memory_map(struct memory_map *memory_map);

void map_address_range(struct memory_map *memory_map,
  uint32_t start, uint32_t length, void *instance,
  memory_rd_function on_read, memory_wr_function on_write);

const struct memory_mapping* resolve_mapped_address(
  const struct memory_map *memory_map, uint32_t address);

#endif

