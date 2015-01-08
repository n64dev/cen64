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
#ifdef _WIN32
  false, // console
#endif
  false, // no_interface
  false, // print_sim_stats
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
    if (!strcmp(argv[i], "-ddipl")) {
      if ((i + 1) >= (argc - 2)) {
        printf("-ddipl requires a path to the ROM file.\n\n");
        return 1;
      }

      options->ddipl_path = argv[++i];
    }

    else if (!strcmp(argv[i], "-ddrom")) {
      if ((i + 1) >= (argc - 2)) {
        printf("-ddrom requires a path to the ROM file.\n\n");
        return 1;
      }

      options->ddrom_path = argv[++i];
    }

    else if (!strcmp(argv[i], "-nointerface"))
      options->no_interface = true;

#ifdef CEN64_DEVFEATURES
    else if (!strcmp(argv[i], "-printsimstats"))
      options->print_sim_stats = true;
#endif

    else
      return 1;
  }

  if (!options->ddipl_path && options->ddrom_path) {
    printf("-ddrom requires a -ddipl argument.\n\n");

    return 1;
  }

  options->pifrom_path = argv[argc - 2];
  options->cart_path = argv[argc - 1];
  return 0;
}

// Prints the command-line usage string.
void print_command_line_usage(const char *invokation_string) {
  printf("%s [Options] <PIF IPL ROM Path> <ROM Path>\n\n"

    "Options:\n"
#ifdef _WIN32
      "  -console                   : Creates/shows the system console.\n"
#endif
      "  -ddipl <path>              : Path to the 64DD IPL ROM (enables 64DD mode).\n"
      "  -ddrom <path>              : Path to the 64DD disk ROM (requires -ddipl).\n"
      "  -nointerface               : Run simulator without a user interface.\n"
#ifdef CEN64_DEVFEATURES
      "  -printsimstats             : Print simulation statistics at exit.\n"
#endif

    ,invokation_string
  );
}

