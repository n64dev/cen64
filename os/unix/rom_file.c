//
// os/unix/rom_file.c
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "os/rom_file.h"
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Unmaps a ROM image from the host address space.
int close_rom_file(const struct rom_file *file) {
  return munmap(file->ptr, file->size);
}

// Maps a ROM into the host address pace, returns a pointer.
int open_rom_file(const char *path, struct rom_file *file) {
  struct stat sb;
  void *ptr;
  int fd;

  /* Open the file for read only. */
  if ((fd = open(path, O_RDONLY)) == -1)
    return -1;

  /* Get the file's size, map it into the address space. */
  if (fstat(fd, &sb) == -1 || (ptr = mmap(NULL,
    sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
    close(fd);

    return -1;
  }

  file->ptr = ptr;
  file->size = sb.st_size;
  file->fd = fd;

  return 0;
}

