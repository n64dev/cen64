//
// protocol.h: gdb message parsing and responding
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef _gdb_protocol_h__
#define _gdb_protocol_h__
#include "common.h"
#include "common/debug_hooks.h"

struct gdb;

cen64_cold void gdb_send_stop_reply(struct gdb* gdb, bool is_breakpoint, enum debug_source source);
cen64_cold void gdb_handle_packet(struct gdb* gdb, const char* command_start, const char* command_end);


#endif
