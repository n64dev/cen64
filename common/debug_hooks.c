
#include "debug_hooks.h"

cen64_cold void debug_init(struct debug_hooks* debug, enum debug_source source) {
  hash_table_init(&debug->breakpoints, 0);
  debug->break_handler = NULL;
  debug->break_handler_data = NULL;
  debug->source = source;
}

cen64_cold void debug_cleanup(struct debug_hooks* debug) {
  hash_table_free(&debug->breakpoints);
}

cen64_cold void debug_check_breakpoints(struct debug_hooks* debug, uint64_t pc) {
  if (debug->break_handler) {
    enum debug_break_reason reason = DEBUG_BREAK_REASON_NONE;
    if (hash_table_get(&debug->breakpoints, (unsigned long)pc, NULL)) {
      reason = DEBUG_BREAK_REASON_BREAKPOINT;
    } else if (debug->signals & DEBUG_SIGNALS_BREAK) {
      reason = DEBUG_BREAK_REASON_PAUSE;
    }
    
    if (reason != DEBUG_BREAK_REASON_NONE) {
      debug->signals &= ~DEBUG_SIGNALS_BREAK;
      debug->break_handler(debug->break_handler_data, reason, debug->source);
    }
  }
}

cen64_cold void debug_exception(struct debug_hooks* debug) {
  if (debug->break_handler) {
    debug->break_handler(debug->break_handler_data, DEBUG_BREAK_REASON_EXCEPTION, debug->source);
  }
}

cen64_cold void debug_set_breakpoint(struct debug_hooks* debug, uint64_t pc) {
  hash_table_set(&debug->breakpoints, (unsigned long)pc, 1);
}

cen64_cold void debug_remove_breakpoint(struct debug_hooks* debug, uint64_t pc) {
  hash_table_delete(&debug->breakpoints, (unsigned long)pc);
}

cen64_cold void debug_signal(struct debug_hooks* debug, enum debug_signals signal) {
  debug->signals |= signal;
}