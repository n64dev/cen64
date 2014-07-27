//
// os/unix/main.c
//
// Entry point for CEN64.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "device.h"
#include "os/main.h"

int main(int argc, const char *argv[]) {
  struct cen64_device device;
  int status;

  status = cen64_main(&device, argc, argv);
  cen64_cleanup(&device);

  return status;
}

