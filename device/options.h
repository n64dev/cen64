//
// options.h: Common CEN64 simulation options.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __options_h__
#define __options_h__
#include "common.h"

struct cen64_options {
  const char *ddipl_path;
  const char *ddrom_path;
  const char *pifrom_path;
  const char *cart_path;
  const char *debugger_addr;

  const char *eeprom_path;
  size_t eeprom_size;
  const char *sram_path;

  struct controller *controller;

#ifdef _WIN32
  bool console;
#endif

  bool enable_debugger;
  bool multithread;
  bool no_audio;
  bool no_video;
};

extern const struct cen64_options default_cen64_options;

cen64_cold int parse_options(struct cen64_options *, int argc, const char *argv[]);
cen64_cold void print_command_line_usage(const char *invokation_string);

#endif

