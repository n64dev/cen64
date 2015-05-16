//
// options.c: CEN64 simulation options.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "options.h"

const struct cen64_options default_cen64_options = {
  NULL, // ddipl_path
  NULL, // ddrom_path
  NULL, // pifrom_path
  NULL, // cart_path
  NULL, // debugger_addr
#ifdef _WIN32
  false, // console
#endif
  false, // enable_debugger
  false, // no_interface
};

// Parses the passed command line arguments.
// Returns nonzero if there was a parse error.
int parse_options(struct cen64_options *options, int argc, const char *argv[]) {
  int i;

  for (i = 0; i < argc - 1; i++) {
#ifdef _WIN32
    if (!strcmp(argv[i], "-console"))
      options->console = true;

    else
#endif

    if (!strcmp(argv[i], "-debug")) {
      options->enable_debugger = true;

      // Check for optional host:port pair.
      if ((i + 1) >= (argc - 1) && argv[i + 1][0] != '-')
        options->debugger_addr = argv[++i];

      else
        options->debugger_addr = "localhost:64646";
    }

    else if (!strcmp(argv[i], "-ddipl")) {
      if ((i + 1) >= (argc - 1)) {
        printf("-ddipl requires a path to the ROM file.\n\n");
        return 1;
      }

      options->ddipl_path = argv[++i];
    }

    else if (!strcmp(argv[i], "-ddrom")) {
      if ((i + 1) >= (argc - 1)) {
        printf("-ddrom requires a path to the ROM file.\n\n");
        return 1;
      }

      options->ddrom_path = argv[++i];
    }

    else if (!strcmp(argv[i], "-nointerface"))
      options->no_interface = true;

    // TODO: Handle this better.
    else
      break;
  }

  // Took this out to permit emulation
  // of the 64DD development package.
#if 0
  if (!options->ddipl_path && options->ddrom_path) {
    printf("-ddrom requires a -ddipl argument.\n\n");

    return 1;
  }
#endif

  options->pifrom_path = argv[i];

  if ((i + 1) < argc)
    options->cart_path = argv[i + 1];

  if (!options->ddipl_path && !options->ddrom_path && !options->cart_path)
    return 1;

  return 0;
}

// Prints the command-line usage string.
void print_command_line_usage(const char *invokation_string) {
#ifdef _WIN32
//  show_console();
#endif

  printf("%s [Options] <PIF IPL ROM Path> [Cart ROM Path]\n\n"

    "Options:\n"
#ifdef _WIN32
      "  -console                   : Creates/shows this system console window.\n"
#endif
      "  -debug [addr][:port]       : Starts the debugger on interface:port.\n"
      "                               By default, CEN64 uses localhost:64646.\n"
      "  -ddipl <path>              : Path to the 64DD IPL ROM (enables 64DD mode).\n"
      "  -ddrom <path>              : Path to the 64DD disk ROM (requires -ddipl).\n"
      "  -nointerface               : Run simulator without a user interface.\n"

    ,invokation_string
  );

#ifdef _WIN32
//  hide_console();
#endif
}

