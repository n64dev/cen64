//
// gdb.h: gdb remote debugger implementation
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef _gdb_h__
#define _gdb_h__
#include "common.h"
#include "vr4300/interface.h"
#include "device/device.h"

#define MAX_GDB_PACKET_SIZE     0x4000

enum gdb_flags {
    GDB_FLAGS_INITIAL = 0x1,
    GDB_FLAGS_CONNECTED = 0x2,
    GDB_FLAGS_PAUSED = 0x4,
    GDB_FLAGS_RSP_SELECTED = 0x8,
};

struct gdb {
    int socket;
    int client;
    struct cen64_device* device;
    int pending_data;
    char packet_buffer[MAX_GDB_PACKET_SIZE*2];
    char output_buffer[MAX_GDB_PACKET_SIZE];
    int flags;
    cen64_thread thread;
    cen64_mutex client_mutex;
    cen64_cv client_semaphore;
};

cen64_cold struct gdb* gdb_alloc();
cen64_cold bool gdb_init(struct gdb* gdb, struct cen64_device* device, const char* host);
cen64_cold void gdb_destroy(struct gdb* gdb);

#endif