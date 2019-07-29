//
// os/posix/alloc.c: Low-level POSIX-based allocators.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/common/alloc.h"
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Global file descriptor for allocations.
static int zero_page_fd = -1;

// Allocates a block of (R/W/X) memory.
void *cen64_alloc(struct cen64_mem *m, size_t size, bool exec) {
  int flags = MAP_PRIVATE;
  int perm = PROT_READ | PROT_WRITE;

  if (exec)
    perm |= PROT_EXEC;

  // Use MAP_ANON on OSX because it really does not
  // enjoy trying to mmap from device files...
#ifdef __APPLE__
  flags |= MAP_ANON;
#endif

  if ((m->ptr = mmap(NULL, size, perm,
    flags, zero_page_fd, 0)) == MAP_FAILED) {
	  perror("mmap");
    return NULL;
  }

#ifdef __APPLE__
  memset(m->ptr, 0, size);
#endif

  m->size = size;
  return m->ptr;
}

// Releases resources acquired by cen64_alloc_init.
void cen64_alloc_cleanup(void) {
#ifndef __APPLE__
  close(zero_page_fd);
#endif
}

// Initializes CEN64's low-level allocator.
int cen64_alloc_init(void) {
#ifndef __APPLE__
  if ((zero_page_fd = open("/dev/zero", O_RDWR)) < 0)
    return -1;
#endif

  return 0;
}

// Releases resources acquired by cen64_alloc.
void cen64_free(struct cen64_mem *m) {
  munmap(m->ptr, m->size);
}

