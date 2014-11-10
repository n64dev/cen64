//
// os/windows/dynarec.c
//
// Functions for allocating executable code buffers.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/dynarec.h"
#include <windows.h>

extern HANDLE dynarec_heap;

// Allocates memory with execute permissions set.
void *alloc_dynarec_slab(struct dynarec_slab *slab, size_t size) {
  if ((slab->ptr = HeapAlloc(dynarec_heap, HEAP_ZERO_MEMORY, size)) == NULL)
    return NULL;

  slab->size = size;
  return slab->ptr;
}

// Frees memory acquired for a dynarec buffer.
void free_dynarec_slab(struct dynarec_slab *slab) {
  HeapFree(dynarec_heap, 0, slab->ptr);
}

