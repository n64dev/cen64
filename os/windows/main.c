//
// os/windows/main.c
//
// Entry point for Unix
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <windows.h>
#include <tchar.h>

int cen64_main(int argc, const char **argv[]);
static WPARAM message_loop(void);

DWORD event_thread_id;

/* Main application event loop. */
WPARAM message_loop(void) {
  MSG msg;

  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

/* Thread in which the engine runs. */
DWORD WINAPI thread_main(LPVOID lpParam) {
  int status;

  event_thread_id = *((DWORD *) lpParam);
  status = cen64_main(__argc, __argv);

  PostThreadMessage(event_thread_id, WM_QUIT, 0, 0);
  return status;
}

/* Windows application entry point. */
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  WSADATA wsa_data = {0};
  HANDLE thread;
  WPARAM status;

  DWORD my_thread_id = GetCurrentThreadId();
  DWORD thread_id;

  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    MessageBox(NULL, L"Failed to initialize WinSock.", L"Error",
      MB_OK | MB_ICONEXCLAMATION);

    return 1;
  }

  if ((thread = CreateThread(
    NULL, 0, thread_main, &my_thread_id, 0, &thread_id)) == NULL) {
    MessageBox(NULL, L"Failed to create the application thread.", L"Error",
      MB_OK | MB_ICONEXCLAMATION);

    WSACleanup();
    return 2;
  }

  status = message_loop();

  WaitForSingleObject(thread, INFINITE);
  CloseHandle(thread);
  WSACleanup();

  return status;
}

