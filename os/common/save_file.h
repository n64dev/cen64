//
// os/save_file.h
//
// Functions for mapping save files into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_save_file_h__
#define __os_save_file_h__
#include "common.h"
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>

struct save_file {
  void *ptr;
  size_t size;
  HANDLE mapping;
  HANDLE file;
};

#else
struct save_file {
  void *ptr;
  size_t size;
  int fd;
};
#endif

cen64_cold int close_save_file(const struct save_file *file);
cen64_cold int open_save_file(const char *path, size_t size, struct save_file *file, int *created);

#endif

