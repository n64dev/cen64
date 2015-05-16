//
// os/windows/main.c
//
// Entry point for CEN64.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"
#include "device/device.h"
#include "device/options.h"
#include "device/netapi.h"
#include "os/common/alloc.h"
#include "os/gl_window.h"
#include "os/main.h"
#include "os/windows/winapi_window.h"
#include <signal.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>

static void hide_console(void);
static void show_console(void);

// Windows application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  WSADATA wsa_data = {0};
  WPARAM status = 255;

  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    MessageBox(NULL, "Failed to initialize Winsock.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return status;
  }

  status = cen64_main(__argc, (const char **) __argv);
  WSACleanup();

  return status;
}

// "Hides" the console window (after waiting for input).
void hide_console(void) {
  printf("\n");
  system("PAUSE");

  FreeConsole();
}

// "Unhides" the console window.
void show_console(void) {
  AllocConsole();

  freopen("CONOUT$", "wb", stdout);
  freopen("CONOUT$", "wb", stderr);
}

