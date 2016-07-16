//
// os/posix/main.c: Entry point for WinAPI backend.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"
#include <winsock2.h>
#include <windows.h>

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

  MessageBox(NULL, "There are performance issues and bugs in this release.\n"
                   "Please adjust the affinity of the CEN64.exe in the task\n"
                   "manager to achieve optimal settings. Most quad core CPU\n"
                   "users will likely find that 0/2/4/6 works best.",

                   "CEN64 Notice", MB_OK | MB_ICONEXCLAMATION);

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

