//
// os/main.h
//
// Convenience functions for managing rendering windows.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_main_h__
#define __os_main_h__
#include "options.h"
#include "os/gl_window.h"
#include "rom_file.h"

int os_main(struct cen64_options *options,
  struct rom_file *pifrom, struct rom_file *cart);

bool os_exit_requested(struct gl_window *gl_window);
void os_render_frame(struct gl_window *gl_window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type);

#endif

