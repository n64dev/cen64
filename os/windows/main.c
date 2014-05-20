//
// os/windows/main.c
//
// Entry point for Windows. 
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "device.h"
#include <windows.h>
#include <tchar.h>

void cen64_cleanup(struct cen64_device *device);
int cen64_main(struct cen64_device *device, int argc, const char *argv[]);

DWORD event_thread_id;
static WPARAM message_loop(void);
static DWORD WINAPI thread_main(LPVOID lpParam);

// Main application event loop.
WPARAM message_loop(void) {
  MSG msg;

  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

// Thread in which the engine runs.
DWORD WINAPI thread_main(LPVOID lpParam) {
  struct cen64_device *device = (struct cen64_device *) lpParam;
  int status;

  status = cen64_main(device, __argc, __argv);
  PostThreadMessage(event_thread_id, WM_QUIT, 0, 0);
  return status;
}

// Windows application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  struct cen64_device device;

  WSADATA wsa_data = {0};
  DWORD thread_id;
  HANDLE thread;
  WPARAM status;

  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    MessageBox(NULL, L"Failed to initialize WinSock.", L"Error",
      MB_OK | MB_ICONEXCLAMATION);

    return 1;
  }

  event_thread_id = GetCurrentThreadId();

  if ((thread = CreateThread(
    NULL, 0, thread_main, &device, 0, &thread_id)) == NULL) {
    MessageBox(NULL, L"Failed to create the application thread.", L"Error",
      MB_OK | MB_ICONEXCLAMATION);

    WSACleanup();
    return 2;
  }

  status = message_loop();
  CloseHandle(thread);

  cen64_cleanup(&device);
  WSACleanup();
  return status;
}

