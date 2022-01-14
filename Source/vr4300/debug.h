//
// vr4300/debug.h: VR4300 debug hooks.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_debug_h__
#define __vr4300_debug_h__
#include "common.h"
#include "common/hash_table.h"
#include "vr4300/interface.h"

enum vr4300_debug_signals {
  VR4300_DEBUG_SIGNALS_BREAK  = 0x000000001,
};

struct vr4300_debug {
    struct hash_table breakpoints;
    vr4300_debug_break_handler break_handler;
    void* break_handler_data;
    unsigned signals;
};

cen64_cold void vr4300_debug_init(struct vr4300_debug* debug);

cen64_cold void vr4300_debug_cleanup(struct vr4300_debug* debug);
cen64_cold void vr4300_debug_check_breakpoints(struct vr4300_debug* debug, uint64_t pc);
cen64_cold void vr4300_debug_exception(struct vr4300_debug* debug);
cen64_cold void vr4300_debug_set_breakpoint(struct vr4300_debug* debug, uint64_t pc);
cen64_cold void vr4300_debug_remove_breakpoint(struct vr4300_debug* debug, uint64_t pc);

cen64_cold void vr4300_debug_signal(struct vr4300_debug* debug, enum vr4300_debug_signals signal);

#endif