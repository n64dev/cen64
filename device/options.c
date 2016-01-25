//
// options.c: CEN64 simulation options.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "options.h"
#include "si/pak.h"

static int parse_controller_options(const char *str, int *num, struct controller *opt);

const struct cen64_options default_cen64_options = {
  NULL, // ddipl_path
  NULL, // ddrom_path
  NULL, // pifrom_path
  NULL, // cart_path
  NULL, // debugger_addr
  NULL, // eeprom_path
  0,    // eeprom_size
  NULL, // sram_path
  { { NULL, }, }, // controller
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

    else if (!strcmp(argv[i], "-eep4k")) {
      if ((i + 1) >= (argc - 1)) {
        printf("-eep4k requires a path to the save file.\n\n");
        return 1;
      }

      options->eeprom_path = argv[++i];
      options->eeprom_size = 0x200; // 4 kbit
    }

    else if (!strcmp(argv[i], "-eep16k")) {
      if ((i + 1) >= (argc - 1)) {
        printf("-eep16k requires a path to the save file.\n\n");
        return 1;
      }

      options->eeprom_path = argv[++i];
      options->eeprom_size = 0x800; // 16 kbit
    }

    else if (!strcmp(argv[i], "-sram")) {
      if ((i + 1) >= (argc - 1)) {
        printf("-sram requires a path to the save file.\n\n");
        return 1;
      }

      options->sram_path = argv[++i];
    }

    else if (!strcmp(argv[i], "-controller")) {
      int num;
      struct controller opt = { NULL, };

      if ((i + 1) >= (argc - 1)) {
        printf("-controller requires a controller description.\n\n");
        return 1;
      }

      if (!parse_controller_options(argv[++i], &num, &opt)) {
        printf("Incorrect option format\n\n");
        return 1;
      }

      options->controller[num] = opt;
    }

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

int parse_controller_options(const char *str, int *num, struct controller *opt) {
  char *token;
  char mempak_path[4096];
  char tpak_rom_path[4096] = { 0, };
  char tpak_save_path[4096] = { 0, };
  char *opt_string = strdup(str);

  if (opt_string == NULL) {
    printf("Unable to dup a string. You're gonna have trouble running games.\n");
    exit(1);
  }
  *num = -1;

  token = strtok(opt_string, ",");
  while (token != NULL) {
    if (sscanf(token, "num=%d", num) == 1)
      ;
    else if (strcmp(token, "pak=rumble") == 0)
      opt->pak = PAK_RUMBLE;
    else if (strcmp(token, "pak=transfer") == 0)
      opt->pak = PAK_TRANSFER;
    else if (sscanf(token, "mempak=%4095s", mempak_path) == 1)
      opt->pak = PAK_MEM;
    else if (sscanf(token, "tpak_rom=%4095s", tpak_rom_path) == 1)
      opt->pak = PAK_TRANSFER;
    else if (sscanf(token, "tpak_save=%4095s", tpak_save_path) == 1)
      opt->pak = PAK_TRANSFER;
    else {
      printf("Unrecognized controller option: %s\n", token);
      goto err;
    }
    // TODO transfer pak options
    token = strtok(NULL, ",");
  }

  if (*num < 1 || *num > 4) {
    printf("Controller number invalid or unspecified.\n");
    goto err;
  }

  --*num; // internally it's used as an index into an array
  mempak_path[4095] = '\0';
  if (strlen(mempak_path) > 0)
    opt->mempak_path = strdup(mempak_path);
  tpak_rom_path[4095] = '\0';
  if (strlen(tpak_rom_path) > 0)
    opt->tpak_rom_path = strdup(tpak_rom_path);
  tpak_save_path[4095] = '\0';
  if (strlen(tpak_save_path) > 0)
    opt->tpak_save_path = strdup(tpak_save_path);
  opt->present = 1;

  free(opt_string);
  return 1;

err:
  free(opt_string);
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
      "  -nointerface               : Run emulator without a user interface.\n"

    ,invokation_string
  );

#ifdef _WIN32
//  hide_console();
#endif
}

