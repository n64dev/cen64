//
// os/winapi/console.c
//
// Functions for manipulating the console.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"
#include <windows.h>

bool has_console(void) {
  DWORD procId;
  DWORD count = GetConsoleProcessList(&procId, 1);
  return (count >= 2);
}

void check_start_from_explorer(void) {
  if (has_console()) return;
  system("cmd /S /K \"echo cen64 is a command-line application, don't double-click on it! & echo: & echo Type ^\"cen64.exe^\" from this prompt to get the usage.\"");
  exit(0);
}


// "Hides" the console window (after waiting for input).
void hide_console(void) {
  printf("\n");
  system("PAUSE");

  FreeConsole();
}

// "Unhides" the console window.
void show_console(void) {
  if (AttachConsole(ATTACH_PARENT_PROCESS) == 0)
    AllocConsole();

  freopen("CONOUT$", "wb", stdout);
  freopen("CONOUT$", "wb", stderr);
}
