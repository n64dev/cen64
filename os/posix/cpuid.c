//
// os/posix/cpuid.c
//
// Functions for calling cpuid on x86.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "os/cpuid.h"

void cen64_cpuid(uint32_t eax, uint32_t ecx, struct cen64_cpuid_t *cpuid) {
  __asm__ __volatile__(
    "cpuid\n\t"

    : "=a"(cpuid->eax), "=b"(cpuid->ebx), "=c"(cpuid->ecx), "=d"(cpuid->edx)
    : "0"(eax), "2"(ecx)
  );  
}

void cen64_cpuid_get_vendor(char vendor[13]) {
  struct cen64_cpuid_t my_cpuid;

  cen64_cpuid(0, 0, &my_cpuid);

  memcpy(vendor + 0, &my_cpuid.ebx, sizeof(my_cpuid.ebx));
  memcpy(vendor + 4, &my_cpuid.edx, sizeof(my_cpuid.edx));
  memcpy(vendor + 8, &my_cpuid.ecx, sizeof(my_cpuid.ecx));
  vendor[sizeof(vendor) - 1] = '\0';
}

