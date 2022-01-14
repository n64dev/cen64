//
// os/winapi/rom_file.c
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "rom_file.h"
#include <stddef.h>
#include <windows.h>

// Unmaps a ROM image from the host address space.
int close_rom_file(const struct rom_file *file) {
  UnmapViewOfFile(file->ptr);
  CloseHandle(file->mapping);
  CloseHandle(file->file);

  return 0;
}

// Maps a ROM into the host address space, returns a pointer.
int open_rom_file(const char *path, struct rom_file *file) {
  void *ptr;
  size_t size;
  HANDLE mapping;
  HANDLE hfile;

  // Open the file, get its size.
  if ((hfile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING,
    FILE_FLAG_RANDOM_ACCESS, NULL)) == INVALID_HANDLE_VALUE)
    return -1;

  size = GetFileSize(hfile, NULL);

  // Create a mapping and effectively enable it.
  if ((mapping = CreateFileMapping(hfile, 0,
    PAGE_READONLY, 0, 0, NULL)) == NULL) {
    CloseHandle(hfile);

    return -2;
  }

  if ((ptr = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0)) == NULL) {
    CloseHandle(mapping);
    CloseHandle(hfile);

    return -3;
  }

  file->ptr = ptr;
  file->size = size;
  file->mapping = mapping;
  file->file = hfile;

  return 0;
}

