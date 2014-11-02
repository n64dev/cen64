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
#ifdef _WIN32
  false, // console
#endif
  false, // extra_mode
};

// Parses the passed command line arguments.
// Returns nonzero if there was a parse error.
int parse_options(struct cen64_options *options, int argc, const char *argv[]) {
  int i;

  for (i = 0; i < argc - 2; i++) {
#ifdef _WIN32
    if (!strcmp(argv[i], "-console"))
      options->console = true;

    else
#endif

    if (!strcmp(argv[i], "-printsimstats"))
      options->extra_mode = true;

    else
      return 1;
  }

  options->pifrom_path = argv[argc - 2];
  options->cart_path = argv[argc - 1];
  return 0;
}

// Prints the command-line usage string.
void print_command_line_usage(const char *invokation_string) {
  printf("%s [Options] <PIFROM Path> <ROM Path>\n\n"

    "Options:\n"
#ifdef _WIN32
      "  -console                   : Creates/shows the system console.\n"
#endif
      "  -printsimstats             : Print simulation statistics at exit.\n",

    invokation_string
  );
}

