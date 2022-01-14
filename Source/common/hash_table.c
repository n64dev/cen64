//
// common/hash_table.c
//
// Simple hash table implementation
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "hash_table.h"

#define LARGE_PRIME 1048583
// must be a power of 2
#define MIN_TABLE_SIZE  16

#define WRAP_INDEX(table, index) ((index) & ((table)->capacity - 1))

struct hash_table_entry* hash_table_find_entry(struct hash_table* table, unsigned long key) {
    if (table->capacity == 0) {
        return NULL;
    }

    int startIndex = WRAP_INDEX(table, key * LARGE_PRIME);
    int currentIndex = startIndex;

    while (table->entries[currentIndex].used) {
        if (table->entries[currentIndex].key == key) {
            return table->entries + currentIndex;
        }

        currentIndex = WRAP_INDEX(table, currentIndex + 1);

        // prevent an infinite loop
        if (currentIndex == startIndex) {
            return NULL;
        }
    }

    return table->entries + currentIndex;
}


void hash_table_alloc(struct hash_table* table, int capacity) {
    if (capacity) {
        table->entries = malloc(sizeof(struct hash_table_entry) * capacity);
    } else {
        table->entries = NULL;
    }

    for (int i = 0; i < capacity; i++) {
        table->entries[i].used = false;
    }

    table->capacity = capacity;
    table->size = 0;
}

void hash_table_resize(struct hash_table* table, int capacity) {
    struct hash_table newTable;
    hash_table_alloc(&newTable, capacity);

    for (int i = 0; i < table->capacity; i++) {
        if (table->entries[i].used) {
            struct hash_table_entry* newEntry = hash_table_find_entry(&newTable, table->entries[i].key);

            newEntry->used = true;
            newEntry->key = table->entries[i].key;
            newEntry->value = table->entries[i].value;
        }
    }

    hash_table_free(table);
    *table = newTable;
}

void hash_table_init(struct hash_table* table, int capacity) {
    int actualCapacity = MIN_TABLE_SIZE;

    // capacity must be a power of 2
    while (actualCapacity < capacity) {
        actualCapacity <<= 1;
    }

    hash_table_alloc(table, actualCapacity);
}

void hash_table_free(struct hash_table* table) {
    free(table->entries);
    table->entries = NULL;
    table->capacity = 0;
    table->size = 0;
}

int hash_table_size(struct hash_table* table) {
    return table->size;
}

int hash_table_capacity(struct hash_table* table) {
    return table->capacity;
}

bool hash_table_get(struct hash_table* table, unsigned long key, unsigned long *value) {
    if (table->capacity == 0) {
        return false;
    }

    struct hash_table_entry* entry = hash_table_find_entry(table, key);
    assert(entry);

    if (entry->used) {
        if (value) {
            *value = entry->value;
        }

        return true;
    } else {
        return false;
    }
}

void hash_table_set(struct hash_table* table, unsigned long key, unsigned long value) {
    if (table->capacity == 0) {
        hash_table_resize(table, MIN_TABLE_SIZE);
    } else if (table->capacity / 2 < table->size) {
        hash_table_resize(table, table->capacity * 2);
    }

    struct hash_table_entry* entry = hash_table_find_entry(table, key);
    assert(entry);

    if (!entry->used) {
        table->size++;
    }

    entry->key = key;
    entry->used = true;
    entry->value = value;
}


void hash_table_check_holes(struct hash_table* table, int startIndex) {
    int index = WRAP_INDEX(table, startIndex + 1);

    while (startIndex != index && table->entries[index].used) {
        struct hash_table_entry* prevEntry = &table->entries[index];
        struct hash_table_entry* newEntry = hash_table_find_entry(table, prevEntry->key);
        assert(newEntry);

        if (newEntry != prevEntry) {
            assert(!newEntry->used);
    
            newEntry->key = prevEntry->key;
            newEntry->value = prevEntry->value;
            newEntry->used = true;

            prevEntry->used = false;
        }

        index = WRAP_INDEX(table, index + 1);
    }
}

void hash_table_delete(struct hash_table* table, unsigned long key) {
    if (table->capacity == 0) {
        return;
    } else if (table->capacity / 4 > table->size && table->capacity > MIN_TABLE_SIZE) {
        hash_table_resize(table, table->capacity / 2);
    }

    struct hash_table_entry* entry = hash_table_find_entry(table, key);
    assert(entry);

    if (entry->used) {
        entry->used = false;
        table->size--;

        // fill in the hole created by deleting this entry
        hash_table_check_holes(table, entry - table->entries);
    }
}
