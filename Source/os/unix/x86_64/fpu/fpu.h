//
// os/unix/x86_64/fpu/fpu.h
//
// Extern declarations for host FPU functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_fpu_h__
#define __os_fpu_h__
#include "common.h"
#include <emmintrin.h>

#include "arch/x86_64/fpu/fpu.h"

#include "os/unix/x86_64/fpu/fpu_cmp_eq_32.h"
#include "os/unix/x86_64/fpu/fpu_cmp_eq_64.h"
#include "os/unix/x86_64/fpu/fpu_cmp_f_32.h"
#include "os/unix/x86_64/fpu/fpu_cmp_f_64.h"
#include "os/unix/x86_64/fpu/fpu_cmp_ole_32.h"
#include "os/unix/x86_64/fpu/fpu_cmp_ole_64.h"
#include "os/unix/x86_64/fpu/fpu_cmp_olt_32.h"
#include "os/unix/x86_64/fpu/fpu_cmp_olt_64.h"
#include "os/unix/x86_64/fpu/fpu_cmp_un_32.h"
#include "os/unix/x86_64/fpu/fpu_cmp_un_64.h"

#endif

