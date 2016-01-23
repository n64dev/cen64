//
// os/unix/save_file.c
//
// Functions for mapping save files into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "save_file.h"
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Unmaps a save file from he host address space.
int close_save_file(const struct save_file *file) {
  return munmap(file->ptr, file->size);
}

// Maps a save into the host address pace, returns a pointer.
int open_save_file(const char *path, size_t size, struct save_file *file) {
  struct stat sb;
  void *ptr;
  int fd;

  // Open the file for read write, and create it if it doesn't exist.
  if ((fd = open(path, O_RDWR | O_CREAT, 0666)) == -1)
    return -1;

  // Get the file's size, map it into the address space.
  if (fstat(fd, &sb) == -1)
    return -1;
  
  if ((size_t)sb.st_size != size)
    if (ftruncate(fd, size) == -1) {
      close(fd);
      return -1;
    }

  if ((ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
    close(fd);
    return -1;
  }

  file->ptr = ptr;
  file->size = size;
  file->fd = fd;

  return 0;
}
