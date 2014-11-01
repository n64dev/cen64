//
// os/unix/main.c
//
// Entry point for CEN64.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"
#include "device.h"
#include "options.h"
#include "os/gl_window.h"
#include "os/main.h"
#include <stdlib.h>

// Unix application entry point.
int main(int argc, const char *argv[]) {
  return cen64_cmdline_main(argc, argv);
}

// Allocates memory for a new device, runs it.
int os_main(struct cen64_options *options,
  struct rom_file *pifrom, struct rom_file *cart) {
  struct gl_window_hints hints;
  int status;

  // Allocate the device on the stack.
  struct cen64_device device;
  memset(&device, 0, sizeof(device));

  // Prevent debugging tools from raising warnings
  // about uninitialized memory being read, etc.
  get_default_gl_window_hints(&hints);

  if (create_gl_window(&device.vi.gl_window, &hints)) {
    printf("Failed to create a window.\n");
    return 1;
  }

  status = device_run(&device, options, malloc(DEVICE_RAMSIZE), pifrom, cart);
  destroy_gl_window(&device.vi.gl_window);

  return status;
}

