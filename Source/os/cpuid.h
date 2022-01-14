//
// os/cpuid.h
//
// Functions for calling cpuid on x86.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_cpuid_h__
#define __os_cpuid_h__
#include "common.h"

struct cen64_cpuid_t {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
};

void cen64_cpuid(uint32_t eax, uint32_t ecx, struct cen64_cpuid_t *cpuid);
void cen64_cpuid_get_vendor(char vendor[13]);

#endif

