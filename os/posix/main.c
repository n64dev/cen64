//
// os/posix/main.c: Entry point for POSIX backend.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"

// Unix application entry point.
int main(int argc, const char *argv[]) {
  return cen64_main(argc, argv);
}

