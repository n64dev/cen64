//
// vr4300/debug.c: VR4300 debug hooks.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "debug.h"

cen64_cold void vr4300_debug_init(struct vr4300_debug* debug) {
  hash_table_init(&debug->breakpoints, 0);
  debug->break_handler = NULL;
  debug->break_handler_data = NULL;
}

cen64_cold void vr4300_debug_cleanup(struct vr4300_debug* debug) {
  hash_table_free(&debug->breakpoints);
}

cen64_cold void vr4300_debug_check_breakpoints(struct vr4300_debug* debug, uint64_t pc) {
  if (debug->break_handler) {
    enum vr4300_debug_break_reason reason = VR4300_DEBUG_BREAK_REASON_NONE;
    if (hash_table_get(&debug->breakpoints, (unsigned long)pc, NULL)) {
      reason = VR4300_DEBUG_BREAK_REASON_BREAKPOINT;
    } else if (debug->signals & VR4300_DEBUG_SIGNALS_BREAK) {
      reason = VR4300_DEBUG_BREAK_REASON_PAUSE;
    }
    
    if (reason != VR4300_DEBUG_BREAK_REASON_NONE) {
      debug->signals &= ~VR4300_DEBUG_SIGNALS_BREAK;
      debug->break_handler(debug->break_handler_data, reason);
    }
  }
}

cen64_cold void vr4300_debug_exception(struct vr4300_debug* debug) {
  if (debug->break_handler) {
    debug->break_handler(debug->break_handler_data, VR4300_DEBUG_BREAK_REASON_EXCEPTION);
  }
}

cen64_cold void vr4300_debug_set_breakpoint(struct vr4300_debug* debug, uint64_t pc) {
  hash_table_set(&debug->breakpoints, (unsigned long)pc, 1);
}

cen64_cold void vr4300_debug_remove_breakpoint(struct vr4300_debug* debug, uint64_t pc) {
  hash_table_delete(&debug->breakpoints, (unsigned long)pc);
}

cen64_cold void vr4300_debug_signal(struct vr4300_debug* debug, enum vr4300_debug_signals signal) {
  debug->signals |= signal;
}