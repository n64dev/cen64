//
// common/debug.h
//
// Verbose debugging functions (read: "fancy print wrappers").
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __common_debug_h__
#define __common_debug_h__

#ifndef NDEBUG
cen64_cold int debug(const char *fmt, ...);
#else
#define debug(...) do {} while (0)
#endif

#endif

