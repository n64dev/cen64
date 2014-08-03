//
// os/windows/main.c
//
// Entry point for CEN64.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "device.h"
#include "os/main.h"
#include <windows.h>
#include <tchar.h>

// Windows application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  struct cen64_device device;
  WPARAM status = 255;

  WSADATA wsa_data = {0};
  HANDLE thread;
  DWORD tid;

  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    MessageBox(NULL, L"Failed to initialize Winsock.", L"CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return ret;
  }

  if (!(status = cen64_main(device, __argc, __argv)))
    cen64_cleanup(&device);

  WSACleanup();
  return ret;
}

