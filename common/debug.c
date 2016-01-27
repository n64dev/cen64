//
// common/debug.c
//
// Verbose debugging functions (read: "fancy print wrappers").
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common/debug.h"
#include "context.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#ifndef NDEBUG

// Writes a formatted string to standard output.
int debug(const char *fmt, ...) {
#ifndef __APPLE__
  struct cen64_context c;
#endif
  va_list ap;
  int ret;

#ifndef __APPLE__
  cen64_context_save(&c);
#endif

  va_start(ap, fmt);
  ret = vfprintf(stdout, fmt, ap);
  va_end(ap);

#ifndef __APPLE__
  cen64_context_restore(&c);
#endif
  return ret;
}

#endif

