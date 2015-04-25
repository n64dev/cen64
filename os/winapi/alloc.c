//
// os/winapi/alloc.h: Low-level WinAPI-based allocators.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/common/alloc.h"
#include <stddef.h>
#include <windows.h>

// Allocates a block of (R/W/X) memory.
void *cen64_alloc(struct cen64_mem *m, size_t size, bool exec) {
  int access = exec ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;

  if ((m->ptr = VirtualAlloc(NULL, size,
    MEM_COMMIT | MEM_RESERVE, access)) == NULL)
    return NULL;

  m->size = size;
  return m->ptr;
}

// Releases resources acquired by cen64_alloc_init.
void cen64_alloc_cleanup(void) {
}

// Initializes CEN64's low-level allocator.
int cen64_alloc_init(void) {
  return 0;
}

// Releases resources acquired by cen64_alloc.
void cen64_free(struct cen64_mem *m) {
  VirtualFree(m->ptr, m->size, MEM_RELEASE);
}

