//
// os/windows/main.c
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
#include <tchar.h>
#include <windows.h>

// Windows application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  WSADATA wsa_data = {0};
  WPARAM status = 255;

  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    MessageBox(NULL, L"Failed to initialize Winsock.", L"CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return status;
  }

  status = cen64_cmdline_main(__argc, __argv);

  WSACleanup();
  return status;
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

