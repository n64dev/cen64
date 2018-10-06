//
// bus/memorymap.c: System memory mapper.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "memorymap.h"

static void fixup(struct memory_map *, struct memory_map_node *);
static void rotate_left(struct memory_map *, struct memory_map_node *);
static void rotate_right(struct memory_map *, struct memory_map_node *);

// Creates a new memory map.
void create_memory_map(struct memory_map *map) {
  memset(map->mappings, 0, sizeof(struct memory_map_node) * 19);
  map->next_map_index = 1;
  map->nil = map->mappings;
  map->root = map->nil;
}

// Rebalances the tree after a node is inserted.
void fixup(struct memory_map *map, struct memory_map_node *node) {
  struct memory_map_node *cur;

  // Rebalance the whole tree as needed. 
  while (node->parent->color == MEMORY_MAP_RED) {
    if (node->parent == node->parent->parent->left) {
      cur = node->parent->parent->right;

      // Case 1: We only need to update colors.
      if (cur->color == MEMORY_MAP_RED) {
        node->parent->color = MEMORY_MAP_BLACK;
        cur->color = MEMORY_MAP_BLACK;
        node->parent->parent->color = MEMORY_MAP_RED;
        node = node->parent->parent;
      }

      else {

        // Case 2: We need to perform a left rotation.
        if (node == node->parent->right) {
          node = node->parent;
          rotate_left(map, node);
        }

        // Case 3: We need to perform a right rotation.
        node->parent->color = MEMORY_MAP_BLACK;
        node->parent->parent->color = MEMORY_MAP_RED;
        rotate_right(map, node->parent->parent);
      }
    }

    else {
      cur = node->parent->parent->left;

      // Case 1: We only need to update colors.
      if (cur->color == MEMORY_MAP_RED) {
        node->parent->color = MEMORY_MAP_BLACK;
        cur->color = MEMORY_MAP_BLACK;
        node->parent->parent->color = MEMORY_MAP_RED;
        node = node->parent->parent;
      }

      else {

        // Case 2: We need to perform a right rotation.
        if (node == node->parent->left) {
          node = node->parent;
          rotate_right(map, node);
        }

        // Case 3: We need to perform a left rotation.
        node->parent->color = MEMORY_MAP_BLACK;
        node->parent->parent->color = MEMORY_MAP_RED;
        rotate_left(map, node->parent->parent);
      }
    }
  }

  // When we rebalanced the tree, we might have accidentally colored
  // the root red, so unconditionally color if back after rebalancing.
  map->root->color = MEMORY_MAP_BLACK;
}

// Inserts a mapping into the tree.
int map_address_range(struct memory_map *map, uint32_t start, uint32_t length,
  void *instance, memory_rd_function on_read, memory_wr_function on_write) {
  struct memory_map_node *check = map->root;
  struct memory_map_node *cur = map->nil;
  uint32_t end = start + length - 1;

  struct memory_map_node *new_node;
  struct memory_mapping mapping;

  // Make sure we have enough space in the map.
  const unsigned num_mappings = sizeof(map->mappings) /
    sizeof(map->mappings[0]) - 1;

  if (unlikely(map->next_map_index >= num_mappings)) {
    debug("map_address_range: Out of free mappings.");
    return 1;
  }

  new_node =  &map->mappings[map->next_map_index++];

  // Walk down the tree.
  while (check != map->nil) {
    cur = check;

    check = (start < cur->mapping.start)
      ? check->left : check->right;
  }

  // Insert the entry.
  if (cur == map->nil)
    map->root = new_node;

  else if (start < cur->mapping.start)
    cur->left = new_node;
  else
    cur->right = new_node;

  new_node->left = map->nil;
  new_node->right = map->nil;
  new_node->parent = cur;

  // Initialize the entry.
  mapping.instance = instance;
  mapping.on_read = on_read;
  mapping.on_write = on_write;

  mapping.end = end;
  mapping.length = length;
  mapping.start = start;

  new_node->mapping = mapping;

  // Rebalance the tree.
  new_node->color = MEMORY_MAP_RED;
  fixup(map, new_node);
  return 0;
}

// Returns a pointer to a region given an address.
const struct memory_mapping *resolve_mapped_address(
  const struct memory_map *map, uint32_t address) {
  const struct memory_map_node *cur = map->root;

  do {
    if (address < cur->mapping.start)
      cur = cur->left;
    else if (address > cur->mapping.end)
      cur = cur->right;

    else
      return &cur->mapping;
  } while (cur != map->nil);

  return NULL;
}

// Performs a left rotation centered at n.
static void rotate_left(struct memory_map *map, struct memory_map_node *n) {
  struct memory_map_node *y = n->right;

  // Turn y's left subtree into n's right subtree.
  n->right = y->left;

  if (y->left != map->nil)
    y->left->parent = n;

  // Link n's parent to y.
  y->parent = n->parent;

  if (n->parent == map->nil)
    map->root = y;
  else if (n == n->parent->left)
    n->parent->left = y;
  else
    n->parent->right = y;

  // Put n on y's left.
  y->left = n;
  n->parent = y;
}

// Performs a right rotation centered at n.
static void rotate_right(struct memory_map *map, struct memory_map_node *n) {
  struct memory_map_node *y = n->left;

  // Turn y's right subtree into n's left subtree.
  n->left = y->right;

  if (y->right != map->nil)
    y->right->parent = n;

  // Link n's parent to y.
  y->parent = n->parent;

  if (n->parent == map->nil)
    map->root = y;
  else if (n == n->parent->left)
    n->parent->left = y;
  else
    n->parent->right = y;

  // Put n on y's right.
  y->right = n;
  n->parent = y;
}

