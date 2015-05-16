//
// os/unix/main.c
//
// Entry point for CEN64.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"
#include "device/device.h"
#include "device/netapi.h"
#include "device/options.h"
#include "os/common/alloc.h"
#include "os/gl_window.h"
#include "os/main.h"
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// Unix application entry point.
int main(int argc, const char *argv[]) {
  return cen64_main(argc, argv);
}

