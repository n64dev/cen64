//
// common/hash_table.h
//
// Simple hash table implementation
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __common_hastable_h__
#define __common_hastable_h__
#include "common.h"

struct hash_table_entry {
    bool used;
    unsigned long key;
    unsigned long value;
};

struct hash_table {
    struct hash_table_entry* entries;
    int capacity;
    int size;
};


void hash_table_init(struct hash_table* table, int capacity);
void hash_table_free(struct hash_table* table);
int hash_table_size(struct hash_table* table);
int hash_table_capacity(struct hash_table* table);

bool hash_table_get(struct hash_table* table, unsigned long key, unsigned long *value);
void hash_table_set(struct hash_table* table, unsigned long key, unsigned long value);
void hash_table_delete(struct hash_table* table, unsigned long key);

#endif

