
#ifndef __common_debug_hooks__
#define __common_debug_hooks__

#include "common.h"
#include "common/hash_table.h"

enum debug_break_reason {
  DEBUG_BREAK_REASON_NONE,
  DEBUG_BREAK_REASON_BREAKPOINT,
  DEBUG_BREAK_REASON_EXCEPTION,
  DEBUG_BREAK_REASON_PAUSE,
};

enum debug_source {
  DEBUG_SOURCE_VR4300,
  DEBUG_SOURCE_RSP,
};

typedef void (*debug_break_handler)(void* data, enum debug_break_reason reason, enum debug_source source);

enum debug_signals {
  DEBUG_SIGNALS_BREAK  = 0x000000001,
};

struct debug_hooks {
    struct hash_table breakpoints;
    debug_break_handler break_handler;
    void* break_handler_data;
    unsigned signals;
    enum debug_source source;
};

void debug_init(struct debug_hooks* debug, enum debug_source source);

void debug_cleanup(struct debug_hooks* debug);
void debug_check_breakpoints(struct debug_hooks* debug, uint64_t pc);
void debug_exception(struct debug_hooks* debug);
void debug_set_breakpoint(struct debug_hooks* debug, uint64_t pc);
void debug_remove_breakpoint(struct debug_hooks* debug, uint64_t pc);

void debug_signal(struct debug_hooks* debug, enum debug_signals signal);


#endif