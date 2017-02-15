//
// os/winapi/cpuid.c
//
// Functions for calling cpuid on x86.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "os/cpuid.h"
#include <intrin.h>


void cen64_cpuid(uint32_t eax, uint32_t ecx, struct cen64_cpuid_t *cpuid) {
  int cpuInfo[4];

  __cpuidex(cpuInfo, eax, ecx);

  cpuid->eax = cpuInfo[0];
  cpuid->ebx = cpuInfo[1];
  cpuid->ecx = cpuInfo[2];
  cpuid->edx = cpuInfo[3];
}

void cen64_cpuid_get_vendor(char vendor[13]) {
  int cpuInfo[4];

  __cpuidex(cpuInfo, 0, 0);

  memcpy(vendor + 0, cpuInfo + 1, sizeof(*cpuInfo));
  memcpy(vendor + 4, cpuInfo + 3, sizeof(*cpuInfo));
  memcpy(vendor + 8, cpuInfo + 2, sizeof(*cpuInfo));
  vendor[sizeof(vendor) - 1] = '\0';
}

