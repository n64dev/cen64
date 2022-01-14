//
// os/dynarec.h
//
// Functions for allocating executable code buffers.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_dynarec_h__
#define __os_dynarec_h__
#include "common.h"
#include <stddef.h>

struct dynarec_slab {
  size_t size;
  uint8_t *ptr;
};

cen64_cold void *alloc_dynarec_slab(struct dynarec_slab *slab, size_t size);
cen64_cold void free_dynarec_slab(struct dynarec_slab *slab);

#endif

