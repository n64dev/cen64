//
// cen64.c: CEN64 entry point.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "bus/controller.h"
#include "common.h"
#include "options.h"
#include "os/main.h"
#include "os/rom_file.h"
#include <stdlib.h>

static int load_roms(const char *pifrom_path, const char *cart_path,
  struct rom_file *pifrom, struct rom_file *cart);

// Called when another simulation instance is desired.
int cen64_cmdline_main(int argc, const char *argv[]) {
	struct cen64_options options = default_cen64_options;
  struct rom_file pifrom, cart;
  int status;

  if (argc < 3) {
    print_command_line_usage(argv[0]);
    return EXIT_SUCCESS;
  }

  if (parse_options(&options, argc - 1, argv + 1)) {
    printf("Invalid command line argument(s) specified.\n");

    print_command_line_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (load_roms(options.pifrom_path, options.cart_path, &pifrom, &cart))
    return EXIT_FAILURE;

  status = os_main(&options, &pifrom, &cart);

  close_rom_file(&cart);
  close_rom_file(&pifrom);
  return status;
}

// Load any ROM images required for simulation.
int load_roms(const char *pifrom_path, const char *cart_path,
  struct rom_file *pifrom, struct rom_file *cart) {
  if (open_rom_file(pifrom_path, pifrom)) {
    printf("Failed to load PIF ROM: %s.\n", pifrom_path);

    return 1;
  }

  if (open_rom_file(cart_path, cart)) {
    printf("Failed to load cart: %s.\n", cart_path);

    close_rom_file(pifrom);
    return 2;
  }

  return 0;
}

