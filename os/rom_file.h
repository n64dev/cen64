//
// os/rom_file.h
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_rom_file_h__
#define __os_rom_file_h__
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>

struct rom_file {
  void *ptr;
  size_t size;
  HANDLE mapping;
  HANDLE file;
};

#else
struct rom_file {
  void *ptr;
  size_t size;
  int fd;
};
#endif

int close_rom_file(const struct rom_file *file);
int open_rom_file(const char *path, struct rom_file *file);

#endif

