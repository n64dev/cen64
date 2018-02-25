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

// Unmaps a save file from the host address space.
int close_save_file(const struct save_file *file) {
  if (file->fd >= 0) {
    munmap(file->ptr, file->size);
    close(file->fd);
  } else {
    free(file->ptr);
  }
  return 0;
}

// Maps a save into the host address space, returns a pointer.
int open_save_file(const char *path, size_t size, struct save_file *file, int *created) {
  struct stat sb;
  void *ptr;
  int fd;
  int my_created;

  if (path == NULL) {
    if ((ptr = calloc(size, 1)) == NULL)
      return -1;
    fd = -1;
    if (created != NULL)
      *created = 1;
  } else {
    // Open the file O_EXCL to see if it exists
    if ((fd = open(path, O_RDWR | O_CREAT | O_EXCL, 0666)) >= 0)
      my_created = 1;

    // Otherwise just open to the file
    else {
      if ((fd = open(path, O_RDWR)) == -1)
        return -1;
      my_created = 0;
    }

    if (created != NULL)
      *created = my_created;

    // Get the file's size, map it into the address space.
    if (fstat(fd, &sb) == -1) {
      close(fd);
      return -1;
    }

    if ((size_t)sb.st_size != size)
      if (ftruncate(fd, size) == -1) {
        close(fd);
        return -1;
      }

    if ((ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
      close(fd);
      return -1;
    }
  }

  file->ptr = ptr;
  file->size = size;
  file->fd = fd;

  return 0;
}

// Opening a game boy save: don't set a specific size and don't create
// the file if it doesn't exist
int open_gb_save(const char *path, struct save_file *file) {
  struct stat sb;
  void *ptr;
  int fd;

  // try to open the file
  if ((fd = open(path, O_RDWR)) == -1)
    return -1;

  // Get the file's size, map it into the address space.
  if (fstat(fd, &sb) == -1) {
    close(fd);
    return -1;
  }

  if ((ptr = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
    close(fd);
    return -1;
  }

  file->ptr = ptr;
  file->size = sb.st_size;
  file->fd = fd;

  return 0;
}
