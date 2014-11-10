//
// os/unix/dynarec.c
//
// Functions for allocating executable code buffers.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/dynarec.h"
#include <sys/mman.h>

extern const int zero_page_fd;

// Allocates memory with execute permissions set.
void *alloc_dynarec_slab(struct dynarec_slab *slab, size_t size) {
  if ((slab->ptr = mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE,
    MAP_PRIVATE, zero_page_fd, 0))  == MAP_FAILED)
    return NULL;

  slab->size = size;
  return slab->ptr;
}

// Frees memory acquired for a dynarec buffer.
void free_dynarec_slab(struct dynarec_slab *slab) {
  munmap(slab->ptr, slab->size);
}

