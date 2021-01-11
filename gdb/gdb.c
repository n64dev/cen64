//
// gdb.c: gdb remote debugger implementation
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "gdb/gdb.h"
#include "gdb/protocol.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

cen64_cold void gdb_handle_breakpoint(void* data, enum vr4300_debug_break_reason reason);

int gdb_read(struct gdb* gdb) {
  if (gdb->pending_data >= MAX_GDB_PACKET_SIZE) {
    // if the gdb client is well behaved this should never happen
    gdb->pending_data = 0;
  }

  int bytes_read = recv(gdb->client, gdb->packet_buffer + gdb->pending_data, MAX_GDB_PACKET_SIZE, 0);

  if (bytes_read > 0) {
    gdb->pending_data += bytes_read;
  }
  

  return bytes_read;
}

void gdb_mark_read(struct gdb* gdb, int handled_bytes) {
  if (handled_bytes <= gdb->pending_data) {
    gdb->pending_data -= handled_bytes;
  } else {
    gdb->pending_data = 0;
  }

  if (gdb->pending_data) {
    for (int i = 0; i < gdb->pending_data; i++) {
      gdb->packet_buffer[i] = gdb->packet_buffer[i + handled_bytes];
    }
  }
}

bool gdb_parse_packet(const char* input, int len, const char** command_start, const char** command_end) {
    const char* string_end = input + len;

    while (input < string_end) {
        if (*input == '$') {
            input++;
            *command_start = input;
            break;
        }
        input++;
    }

    while (input < string_end) {
        if (*input == '#') {
            *command_end = input;
            return true;
        } else {
            input++;
        }
    }

    return false;
}

CEN64_THREAD_RETURN_TYPE gdb_thread(void *opaque) {
  cen64_thread_setname(NULL, "gdb");
  struct gdb *gdb = (struct gdb *) opaque;

  cen64_mutex_lock(&gdb->client_mutex);

  // wait until first breakpoint is hit before entering loop
  if (gdb->flags & GDB_FLAGS_INITIAL) {
    cen64_cv_wait(&gdb->client_semaphore, &gdb->client_mutex);
  } else {
    cen64_mutex_lock(&gdb->client_mutex);
  }

  while (gdb->flags & GDB_FLAGS_CONNECTED) {
    int bytes_read = gdb_read(gdb);

    debug("rec: %.*s\n", bytes_read, gdb->packet_buffer + gdb->pending_data - bytes_read);

    if (bytes_read <= 0) {
      gdb->flags &= ~GDB_FLAGS_CONNECTED;
      printf("Debug socket closed\n");
      break;
    }

    const char* command_start;
    const char* command_end;

    bool did_handle = false;

    do {
      int handled_bytes = 0;

      int search_end = gdb->pending_data;

      if (gdb_parse_packet(gdb->packet_buffer, gdb->pending_data, &command_start, &command_end)) {
        send(gdb->client, "+", strlen("+"), 0);
        gdb_handle_packet(gdb, command_start, command_end);
        
        // +3, 1 byte for the '#' and 2 additional bytes for the checksum
        handled_bytes = (command_end - gdb->packet_buffer) + 3;
        search_end = command_start - gdb->packet_buffer;
      }

      int at;

      for (at = 0; at < search_end && gdb->packet_buffer[at] != '$'; at++) {
        if (gdb->packet_buffer[at] == 0x03) {
          vr4300_signal_break(gdb->device->vr4300);
        }
      }

      if (at > handled_bytes) {
        handled_bytes = at;
      }

      bytes_read -= handled_bytes;
      gdb_mark_read(gdb, handled_bytes);
    } while (did_handle);

  }

  return CEN64_THREAD_RETURN_VAL;
}

cen64_cold void gdb_handle_breakpoint(void* data, enum vr4300_debug_break_reason reason) {
  struct gdb* gdb = (struct gdb*)data;

  debug("Stopping at 0x%08x\n", (uint32_t)vr4300_get_pc(gdb->device->vr4300));

  if (!(gdb->flags & GDB_FLAGS_CONNECTED)) {
    return;
  } else if (gdb->flags & GDB_FLAGS_INITIAL) {
    gdb->flags &= ~GDB_FLAGS_INITIAL;
    vr4300_remove_breakpoint(gdb->device->vr4300, 0xFFFFFFFF80000000ULL);
    cen64_cv_signal(&gdb->client_semaphore);
  } else {
    gdb_send_stop_reply(gdb, reason == VR4300_DEBUG_BREAK_REASON_BREAKPOINT);
  }

  cen64_mutex_lock(&gdb->client_mutex);
  gdb->flags |= GDB_FLAGS_PAUSED;
  cen64_cv_wait(&gdb->client_semaphore, &gdb->client_mutex);
  gdb->flags &= ~GDB_FLAGS_PAUSED;

  if (!(gdb->flags & GDB_FLAGS_CONNECTED)) {
    vr4300_connect_debugger(gdb->device->vr4300, NULL, NULL);
  }
}

cen64_cold struct gdb* gdb_alloc() {
    struct gdb* result = malloc(sizeof(struct gdb));
    memset(result, 0, sizeof(struct gdb));
    return result;
}

cen64_cold bool gdb_init(struct gdb* gdb, struct cen64_device* device, const char* host) {
  int port;
  if (sscanf(host, "localhost:%d", &port) != 1) {
    printf("debugger error: -debug flag must be followed by locahost:<port number>\n");
    return false;
  }

  if (port < 0 || port > 0xffff) {
    printf("debugger error: -debug port must be a value between 0-65535\n");
    return false;
  }

  if ((gdb->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    printf("debugger error: could create socket\n");
    return false;
  }
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  //addr.sin_addr.s_addr = IPADDR_ANY;
  addr.sin_port = htons((unsigned short)port);

  int flag = 1;  
  if (-1 == setsockopt(gdb->socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
    printf("debugger error: could not open port %d\n", (int)port);
    return false;
  }

  if(bind(gdb->socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    printf("debugger error: could not open port %d\n", (int)port);
    return false;
  }

  if(listen(gdb->socket, 1) < 0) {
    printf("debugger error: could not listen on port %d\n", (int)port);
    return false;
  }

  printf("Waiting for gdb connection on port %d\n", (int)port);

  if((gdb->client = accept(gdb->socket, NULL, NULL)) < 0) {
    printf("debugger error: could not connect to client on port %d\n", (int)port);
    return false;
  }

  gdb->device = device;
  device_connect_debugger(device, gdb, &gdb_handle_breakpoint);

  gdb->flags = GDB_FLAGS_INITIAL | GDB_FLAGS_CONNECTED;
  vr4300_set_breakpoint(device->vr4300, 0xFFFFFFFF80000000ULL);

  if (cen64_mutex_create(&gdb->client_mutex)) {
    printf("Failed to create gdb client semaphore.\n");
    return false;
  }

  if (cen64_cv_create(&gdb->client_semaphore)) {
    cen64_mutex_destroy(&gdb->client_mutex);
    printf("Failed to create gdb client semaphore.\n");
    return false;
  }

  if (cen64_thread_create(&gdb->thread, gdb_thread, gdb)) {
    cen64_mutex_destroy(&gdb->client_mutex);
    cen64_cv_destroy(&gdb->client_semaphore);
    printf("Failed to create gdb thread.\n");
    return false;
  }

  return true;
}

cen64_cold void gdb_destroy(struct gdb* gdb) {
  gdb->flags = 0;

  shutdown(gdb->client, 2);
  shutdown(gdb->socket, 2);

  cen64_thread_join(&gdb->thread);
  cen64_mutex_destroy(&gdb->client_mutex);
  cen64_cv_destroy(&gdb->client_semaphore);

  gdb->device = NULL;
  free(gdb);
}