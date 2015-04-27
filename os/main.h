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
#include "device/device.h"
#include "device/options.h"
#include "os/gl_window.h"
#include "rom_file.h"

cen64_cold int os_main(struct cen64_device *device,
  struct cen64_options *options);

cen64_cold bool os_exit_requested(struct gl_window *gl_window);
cen64_cold void os_render_frame(cen64_gl_window window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type);

cen64_cold void os_acquire_input(struct gl_window *gl_window);
cen64_cold void os_release_input(struct gl_window *gl_window);

#endif

