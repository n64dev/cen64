//
// protocol.c: gdb message parsing and responding
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "gdb/protocol.h"
#include "gdb/gdb.h"
#include "vr4300/cpu.h"
#include "rsp/interface.h"

#include <inttypes.h>

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

#define GDB_GLOBAL_THREAD_ID   1
#define GDB_RSP_THEAD_ID      2

#define GDB_GET_EXC_CODE(cause) (((cause) >> 2) & 0x1f)
#define GDB_TRANSLATE_PC(pc)    ((uint64_t)(pc) | 0xFFFFFFFF00000000ULL)
#define GDB_STR_STARTS_WITH(str, const_str) (strncmp(str, const_str, sizeof const_str - 1) == 0)

#define GDB_RSP_MEMORY_BASE  0x04000000
#define GDB_RSP_INSTRUCTION_BASE  0x04001000
#define GDB_RSP_INSTRUCTION_END  0x04002000
#define GDB_RSP_INSTRUCTION_SIZE 0x1000

// not standard just used to allow GDB to access vector registers
#define GDB_RSP_VREG_BASE 0x04020000
#define GDB_RSP_VREG_LEN   512

static int gdb_signals[32] = {
  2, // SIGINT
  11, // SIGSEGV
  11, // SIGSEGV
  11, // SIGSEGV
  11, // SIGSEGV
  11, // SIGSEGV
  10, // SIGBUS
  10, // SIGBUS
  12, // SIGSYS
  5, // SIGTRAP
  4, // SIGILL
  30, // SIGUSR1
  8, // SIGFPE
  5, // SIGTRAP
  0, // reserved
  8, // SIGFPE
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  5, // SIGTRAP
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
  0, // reserved
};

bool gdb_is_rsp_addr(uint64_t addr) {
  return addr >= GDB_RSP_MEMORY_BASE && addr < GDB_RSP_INSTRUCTION_END;
}

bool gdb_is_rsp_instruction(uint64_t addr) {
  return (addr & 0xFFFFFFFF) < GDB_RSP_INSTRUCTION_SIZE;
}

bool gdb_is_rsp_register(uint64_t addr) {
  return addr >= GDB_RSP_VREG_BASE && addr < GDB_RSP_VREG_BASE + GDB_RSP_VREG_LEN;
}

void gdb_set_breakpoint(struct cen64_device* device, uint64_t addr) {
  if (gdb_is_rsp_instruction(addr)) {
    rsp_set_breakpoint(&device->rsp, addr & 0xFFF);
  } else {
    vr4300_set_breakpoint(device->vr4300, addr);
  }
}

void gdb_remove_breakpoint(struct cen64_device* device, uint64_t addr) {
  if (gdb_is_rsp_instruction(addr)) {
    rsp_remove_breakpoint(&device->rsp, addr & 0xFFF);
  } else {
    vr4300_remove_breakpoint(device->vr4300, addr);
  }
}

int gdb_read_hex_digit(char character) {
  if (character >= 'a' && character <= 'f') {
    return 10 + character - 'a';
  } else if (character >= 'A' && character <= 'F') {
    return 10 + character - 'A';
  } else if (character >= '0' && character <= '9') {
    return character - '0';
  } else {
    return -1;
  }   
}

uint32_t gdb_parse_hex(const char* src, int max_bytes) {
  uint32_t result = 0;
  int current_char;
  int max_characters = max_bytes * 2;

  for (current_char = 0; current_char < max_characters; ++current_char) {
    int digit = gdb_read_hex_digit(*src);

    if (digit != -1) {
      result = (result << 4) + digit;
    } else {
      break;
    }

    ++src;
  }

  return result;
}

static char gdb_hex_letters[16] = "0123456789abcdef";

char* gdb_write_hex64(char* target, uint64_t data, int data_size) {
  int shift = data_size * 8 - 4;

  while (shift >= 0) {
    *target++ = gdb_hex_letters[(data >> shift) & 0xF];
    shift -= 4;
  }

  return target;
}

char* gdb_write_hexstring(char* target, const char* src) {
  while (*src) {
    *target++ = gdb_hex_letters[(*src >> 4) & 0xF];
    *target++ = gdb_hex_letters[(*src >> 0) & 0xF];
    src++;
  }

  return target;

}

int gdb_apply_checksum(char* message) {
    char* message_start = message;
    if (*message == '$') {
        ++message;
    }
    
    unsigned char checksum = 0;
    while (*message)
    {
        if (*message == '#') {
            ++message;
            break;
        }

        checksum += (unsigned char)*message;
        ++message;
    }

    sprintf(message, "%02x", checksum);

    return (message - message_start) + 2;
}

void gdb_send(struct gdb* gdb) {
  int messageLen = gdb_apply_checksum(gdb->output_buffer);
  send(gdb->client, gdb->output_buffer, messageLen, 0);
  debug("send: %.*s\n", messageLen, gdb->output_buffer);
}

void gdb_send_literal(struct gdb* gdb, const char* message) {
    send(gdb->client, message, strlen(message), 0);
    debug("send: %s\n", message);
}

void gdb_handle_query(struct gdb* gdb, const char* command_start, const char *command_end) {
  if (GDB_STR_STARTS_WITH(command_start, "qSupported")) {
    strcpy(gdb->output_buffer, "$PacketSize=4000;vContSupported+;swbreak+#");
    gdb_send(gdb);
  } else if (GDB_STR_STARTS_WITH(command_start, "qTStatus")) {
    gdb_send_literal(gdb, "$#00");
  } else if (GDB_STR_STARTS_WITH(command_start, "qfThreadInfo")) {
    sprintf(gdb->output_buffer, "$m%x#", GDB_GLOBAL_THREAD_ID);
    gdb_send(gdb);
  } else if (GDB_STR_STARTS_WITH(command_start, "qsThreadInfo")) {
    strcpy(gdb->output_buffer, "$l#");
    gdb_send(gdb);
  } else if (GDB_STR_STARTS_WITH(command_start, "qAttached")) {
    strcpy(gdb->output_buffer, "$0#");
    gdb_send(gdb);
  } else if (GDB_STR_STARTS_WITH(command_start, "qC")) {
    sprintf(gdb->output_buffer, "$QC%x#", GDB_GLOBAL_THREAD_ID);
    gdb_send(gdb);
  } else if (GDB_STR_STARTS_WITH(command_start, "qTfV")) {
    gdb_send_literal(gdb, "$#00");
  } else if (GDB_STR_STARTS_WITH(command_start, "qTfP")) {
    gdb_send_literal(gdb, "$#00");
  } else if (GDB_STR_STARTS_WITH(command_start, "qOffsets")) {
    sprintf(gdb->output_buffer, "$Text=%x;Data=%x;Bss=%x#", 0, 0, 0);
    gdb_send(gdb);
  } else if (GDB_STR_STARTS_WITH(command_start, "qSymbol")) {
    gdb_send_literal(gdb, "$OK#9a");
  } else if (GDB_STR_STARTS_WITH(command_start, "qThreadExtraInfo")) {
    strcpy(gdb->output_buffer, "$746872656164#");
    gdb_send(gdb);
  } else {
    gdb_send_literal(gdb, "$#00");
  }
}

void gdb_handle_v(struct gdb* gdb, const char* command_start, const char *command_end) {
  if (GDB_STR_STARTS_WITH(command_start, "vMustReplyEmpty")) {
    gdb_send_literal(gdb, "$#00");
  } else if (GDB_STR_STARTS_WITH(command_start, "vCont")) {
    if (command_start[5] == '?') {
      strcpy(gdb->output_buffer, "$c;t#");
      gdb_send(gdb);
    } else {
      switch (command_start[6])
      {
        case 'c':
          cen64_cv_signal(&gdb->client_semaphore);
          break;
        case 't':
          vr4300_signal_break(gdb->device->vr4300);
          break;
      }
    }
  } else if (GDB_STR_STARTS_WITH(command_start, "vKill")) {
    gdb->flags &= ~GDB_FLAGS_CONNECTED;
    gdb->flags &= ~GDB_FLAGS_PAUSED;
    cen64_cv_signal(&gdb->client_semaphore);
    gdb_send_literal(gdb, "$OK#9a");
  } else {
    gdb_send_literal(gdb, "$#00");
  }
}

void gdb_reply_vr4300_registers(struct gdb* gdb) {
  char* current = gdb->output_buffer;
  *current++ = '$';

  // R0
  current = gdb_write_hex64(current, 0, sizeof(uint64_t));
  for (int i = VR4300_REGISTER_AT; i <= VR4300_REGISTER_RA; i++) {
    current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, i), sizeof(uint64_t));
  }

  current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, VR4300_CP0_REGISTER_STATUS), sizeof(uint64_t));
  current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, VR4300_REGISTER_LO), sizeof(uint64_t));
  current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, VR4300_REGISTER_HI), sizeof(uint64_t));
  current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, VR4300_CP0_REGISTER_BADVADDR), sizeof(uint64_t));
  current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, VR4300_CP0_REGISTER_CAUSE), sizeof(uint64_t));
  current = gdb_write_hex64(current, vr4300_get_pc(gdb->device->vr4300), sizeof(uint64_t));

  for (int i = VR4300_REGISTER_CP1_0; i <= VR4300_REGISTER_CP1_31; i++) {
    current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, i), sizeof(uint64_t));
  }

  current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, VR4300_CP1_FCR31), sizeof(uint64_t));

  *current++ = '#';
  *current++ = '\0';

  gdb_send(gdb);
}

void gdb_reply_rsp_registers(struct gdb* gdb) {
  char* current = gdb->output_buffer;
  *current++ = '$';

  // R0
  current = gdb_write_hex64(current, 0, sizeof(uint64_t));
  for (int i = RSP_REGISTER_AT; i <= RSP_REGISTER_RA; i++) {
    current = gdb_write_hex64(current, rsp_get_register(&gdb->device->rsp, i), sizeof(uint64_t));
  }

  current = gdb_write_hex64(current, 0, sizeof(uint64_t)); // VR4300_CP0_REGISTER_STATUS
  current = gdb_write_hex64(current, 0, sizeof(uint64_t)); // VR4300_REGISTER_LO
  current = gdb_write_hex64(current, 0, sizeof(uint64_t)); // VR4300_REGISTER_HI
  current = gdb_write_hex64(current, 0, sizeof(uint64_t)); // VR4300_CP0_REGISTER_BADVADDR
  current = gdb_write_hex64(current, 0, sizeof(uint64_t)); // VR4300_CP0_REGISTER_CAUSE
  current = gdb_write_hex64(current,  rsp_get_pc(&gdb->device->rsp), sizeof(uint64_t));

  for (int i = VR4300_REGISTER_CP1_0; i <= VR4300_REGISTER_CP1_31; i++) {
    current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, i), sizeof(uint64_t));
  }

  current = gdb_write_hex64(current, vr4300_get_register(gdb->device->vr4300, VR4300_CP1_FCR31), sizeof(uint64_t));

  *current++ = '#';
  *current++ = '\0';

  gdb_send(gdb);
}

void gdb_reply_registers(struct gdb* gdb) {
  if (gdb->flags & GDB_FLAGS_RSP_SELECTED) {
    gdb_reply_rsp_registers(gdb);
  } else {
    gdb_reply_vr4300_registers(gdb);
  }
}

void gdb_reply_memory(struct gdb* gdb, const char* command_start, const char *command_end) {
  char* current = gdb->output_buffer;
  *current++ = '$';

  const char* len_text = command_start + 1;

  while (*len_text != ',' && len_text != command_end) {
      ++len_text;
  }

  int32_t addr = gdb_parse_hex(command_start + 1, 4);
  int32_t len = gdb_parse_hex(len_text + 1, 4);

  int32_t alignedAddr = addr & ~0x3;
  int32_t byteShift = addr - alignedAddr;
    
  for (int curr = 0; curr < len + byteShift; curr += 4) {
    uint32_t word = 0;
    int32_t vaddr = alignedAddr + curr;

    if ((gdb->flags & GDB_FLAGS_RSP_SELECTED) && gdb_is_rsp_instruction(vaddr)) {
      bus_read_word(&gdb->device->bus, vaddr + GDB_RSP_INSTRUCTION_BASE, &word);
    } else if (gdb_is_rsp_register(vaddr)) {
      int32_t relative = (vaddr - GDB_RSP_VREG_BASE) / sizeof(uint16_t);
      int32_t registerIndex = relative / 8;
      int32_t elementIndex = relative % 8;

      word = ((int32_t)gdb->device->rsp.cp2.regs[registerIndex].e[elementIndex] << 16) | 
        (int32_t)gdb->device->rsp.cp2.regs[registerIndex].e[elementIndex + 1];
    } else if (gdb_is_rsp_addr(vaddr)) {
      bus_read_word(&gdb->device->bus, vaddr, &word);
    } else {
      vr4300_read_word_vaddr(gdb->device->vr4300, vaddr, &word);
    }

    current = gdb_write_hex64(current, word, sizeof(uint32_t));
  }

  if (byteShift) {
    for (int curr = 0; curr < len * 2; curr++) {
      gdb->output_buffer[curr + 1] = gdb->output_buffer[curr + 1 + byteShift * 2];
    }
  }

  current = gdb->output_buffer + 1 + len * 2;

  *current++ = '#';
  *current++ = '\0';

  gdb_send(gdb);
}

void gdb_handle_packet(struct gdb* gdb, const char* command_start, const char* command_end) {
  switch (*command_start) {
    case 'q':
      gdb_handle_query(gdb, command_start, command_end);
      break;
    case 'v':
      gdb_handle_v(gdb, command_start, command_end);
      break;
    case 'H':
      gdb_send_literal(gdb, "$OK#9a");
      break;
    case '!':
      gdb_send_literal(gdb, "$#00");
      break;
    case '?':
      gdb_send_stop_reply(gdb, false, DEBUG_SOURCE_VR4300);
      break;
    case 'g':
      gdb_reply_registers(gdb);
      break;
    case 'm':
      gdb_reply_memory(gdb, command_start, command_end);
      break;
    case 'D':
      gdb->flags &= ~GDB_FLAGS_CONNECTED;
      gdb_send_literal(gdb, "$OK#9a");;
      break;
    case 'z':
    case 'Z':
    {
      if (command_start[1] == '0') {
        uint64_t addr = GDB_TRANSLATE_PC(gdb_parse_hex(&command_start[3], 4));

        if (*command_start == 'z') {
          gdb_remove_breakpoint(gdb->device, addr);
        } else {
          gdb_set_breakpoint(gdb->device, addr);
        }

        return gdb_send_literal(gdb, "$OK#9a");
      } else {
        gdb_send_literal(gdb, "$#00");
      }
      break;
    }
    default:
      gdb_send_literal(gdb, "$#00");
  }
}

cen64_cold void gdb_send_stop_reply(struct gdb* gdb, bool is_breakpoint, enum debug_source source) {
  char* current = gdb->output_buffer;
  int exc_code;

  if (source == DEBUG_SOURCE_RSP) {
    gdb->flags |= GDB_FLAGS_RSP_SELECTED;
    exc_code = 9;
  } else {
    gdb->flags &= ~GDB_FLAGS_RSP_SELECTED;
    if (is_breakpoint) {
      exc_code = 9;
    } else {
      exc_code = GDB_GET_EXC_CODE(vr4300_get_register(gdb->device->vr4300, VR4300_CP0_REGISTER_CAUSE));
    }
  }

  current += sprintf(current, "$T%02x", gdb_signals[exc_code]);
  if (is_breakpoint) {
    current += sprintf(current, "swbreak:");
  }
  current += sprintf(current, "thread:%d;", GDB_GLOBAL_THREAD_ID);
  *current++ = '#';
  *current++ = '\0';

  gdb_send(gdb);
}