//
// os/winapi/console.h: Console manipulation functions
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//
#ifndef CEN64_OS_WINAPI_CONSOLE
#define CEN64_OS_WINAPI_CONSOLE

void show_console(void);
void hide_console(void);
bool has_console(void);
void check_start_from_explorer(void);

#endif
