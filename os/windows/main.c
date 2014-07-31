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

DWORD event_thread_id;

static WPARAM message_loop(void);
static DWORD WINAPI thread_main(LPVOID lpParam);

// Thread in which the engine runs.
static DWORD WINAPI ThreadMain(LPVOID lpParam) {
  struct cen64_device *device = (struct cen64_device *) lpParam;

  int status = cen64_main(device, __argc, __argv);
  return PostThreadMessage(event_thread_id, WM_QUIT, status, 0);
}

// Windows application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  struct cen64_device device;
  WPARAM ret = 255;

  WSADATA wsa_data = {0};
  HANDLE thread;
  DWORD tid;

  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    MessageBox(NULL, L"Failed to initialize Winsock.", L"CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return ret;
  }

  event_thread_id = GetCurrentThreadId();

  if ((thread = CreateThread(NULL, 0, ThreadMain, &device, 0, &tid)) == NULL) {
    MessageBox(NULL, L"Failed to create the application thread.", L"CEN64",
      MB_OK | MB_ICONEXCLAMATION);
  }

  else {
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    ret = msg.wParam;
  }

  CloseHandle(thread);

  if (!ret)
    cen64_cleanup(&device);

  WSACleanup();
  return ret;
}

