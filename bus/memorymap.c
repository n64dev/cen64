//
// bus/memorymap.c: System memory mapper.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
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
struct memory_map *create_memory_map(unsigned num_maps) {
  struct memory_map_node *mappings;
	struct memory_map *map;

	size_t alloc_size = sizeof(*map) + sizeof(*mappings) * (num_maps + 1);
	if ((map = (struct memory_map*) malloc(alloc_size)) == NULL)
    return NULL;

	mappings = (struct memory_map_node*) (map + 1);

	// Initialize the allocation.
	memset(map, 0, alloc_size);
	map->mappings = mappings;

  // Initialize the tree.
  map->num_mappings = num_maps;
	map->nil = &mappings[num_maps];
  map->root = map->nil;

  return map;
}

// Releases memory allocated for a memory map.
void destroy_memory_map(struct memory_map *memory_map) {
  free(memory_map);
}

// Rebalances the tree after a node is inserted.
static void fixup(struct memory_map *map, struct memory_map_node *node) {
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
void map_address_range(struct memory_map *map, uint32_t start, uint32_t length,
  void *instance, memory_function on_read, memory_function on_write) {
  struct memory_map_node *check = map->root;
  struct memory_map_node *cur = map->nil;
	uint32_t end = start + length - 1;

  struct memory_map_node *newNode;
	struct memory_mapping mapping;

  // Make sure we have enough space in the map.
  assert(map->next_map_index < map->num_mappings &&
    "Tried to insert into a memory_map with no free mappings.");

  newNode =  &map->mappings[map->next_map_index++];

  // Walk down the tree.
  while (check != map->nil) {
    cur = check;

    check = (start < cur->mapping.start)
      ? check->left : check->right;
  }

	// Insert the entry.
  if (cur == map->nil)
    map->root = newNode;

  else if (start < cur->mapping.start)
    cur->left = newNode;
  else
    cur->right = newNode;

  newNode->left = map->nil;
  newNode->right = map->nil;
  newNode->parent = cur;

	// Initialize the entry.
	mapping.instance = instance;
	mapping.on_read = on_read;
	mapping.on_write = on_write;

	mapping.end = end;
	mapping.length = length;
	mapping.start = start;

	newNode->mapping = mapping;

	// Rebalance the tree.
  newNode->color = MEMORY_MAP_RED;
	fixup(map, newNode);
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

