//
// os/winapi/save_file.c
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "save_file.h"
#include <stddef.h>
#include <windows.h>

// Unmaps a save image from the host address space.
int close_save_file(const struct save_file *file) {
  UnmapViewOfFile(file->ptr);
  CloseHandle(file->mapping);
  CloseHandle(file->file);

  return 0;
}

// Maps a ROM into the host address space, returns a pointer.
int open_save_file(const char *path, size_t size,
  struct save_file *file, int *created) {
  void *ptr;
  HANDLE mapping;
  HANDLE hfile;
  LARGE_INTEGER sz;
  int my_created;

  // Open the file, get its size.
  if ((hfile = CreateFile(path, GENERIC_READ | GENERIC_WRITE,
    0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL))
    != INVALID_HANDLE_VALUE)
    my_created = 0;

  else {
    if ((hfile = CreateFile(path, GENERIC_READ | GENERIC_WRITE,
      0, NULL, CREATE_NEW, FILE_FLAG_RANDOM_ACCESS, NULL))
      == INVALID_HANDLE_VALUE)
      return -1;
    my_created = 1;
  }

  if (created != NULL)
    *created = my_created;

  sz.QuadPart = size;
  SetFilePointerEx(hfile, sz, 0, FILE_BEGIN);
  SetEndOfFile(hfile);

  // Create a mapping and effectively enable it.
  if ((mapping = CreateFileMapping(hfile, 0,
    PAGE_READWRITE, 0, 0, NULL)) == NULL) {
    CloseHandle(hfile);

    return -2;
  }

  if ((ptr = MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE,
    0, 0, 0)) == NULL) {
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

