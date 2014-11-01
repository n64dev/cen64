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
#include "rom_file.h"

int os_main(struct cen64_options *options,
  struct rom_file *pifrom, struct rom_file *cart);

#endif

