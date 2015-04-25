//
// os/common/alloc.h: Low-level allocators.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_COMMON_ALLOC
#define CEN64_OS_COMMON_ALLOC
#include "common.h"

struct cen64_mem {
  size_t size;
  void *ptr;
};

cen64_cold void cen64_alloc_cleanup(void);
cen64_cold int cen64_alloc_init(void);

cen64_cold void *cen64_alloc(struct cen64_mem *m, size_t size, bool exec);
cen64_cold void cen64_free(struct cen64_mem *m);

#endif

