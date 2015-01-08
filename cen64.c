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
#include "device/options.h"
#include "os/main.h"
#include "os/rom_file.h"
#include <stdlib.h>

static int load_roms(const char *ddipl_path, const char *ddrom_path,
  const char *pifrom_path, const char *cart_path, struct rom_file *ddipl,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart);

// Called when another simulation instance is desired.
int cen64_cmdline_main(int argc, const char *argv[]) {
	struct cen64_options options = default_cen64_options;
  struct rom_file ddipl, ddrom, pifrom, cart;
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

  if (load_roms(options.ddipl_path, options.ddrom_path, options.pifrom_path,
    options.cart_path, &ddipl, &ddrom, &pifrom, &cart))
    return EXIT_FAILURE;

  status = os_main(&options, &ddipl, &ddrom, &pifrom, &cart);

  if (options.ddipl_path)
    close_rom_file(&ddipl);

  if (options.ddrom_path)
    close_rom_file(&ddrom);

  if (options.cart_path)
    close_rom_file(&cart);

  close_rom_file(&pifrom);
  return status;
}

// Load any ROM images required for simulation.
int load_roms(const char *ddipl_path, const char *ddrom_path,
  const char *pifrom_path, const char *cart_path, struct rom_file *ddipl,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart) {
  memset(ddipl, 0, sizeof(*ddipl));

  if (ddipl_path && open_rom_file(ddipl_path, ddipl)) {
    printf("Failed to load DD IPL ROM: %s.\n", ddipl_path);

    return 1;
  }

  if (ddrom_path && open_rom_file(ddrom_path, ddrom)) {
    printf("Failed to load DD ROM: %s.\n", ddrom_path);

    if (ddipl_path)
      close_rom_file(ddipl);

    return 2;
  }

  if (open_rom_file(pifrom_path, pifrom)) {
    printf("Failed to load PIF ROM: %s.\n", pifrom_path);

    if (ddipl_path)
      close_rom_file(ddipl);

    if (ddrom_path)
      close_rom_file(ddrom);

    return 3;
  }

  if (cart_path && open_rom_file(cart_path, cart)) {
    printf("Failed to load cart: %s.\n", cart_path);

    if (ddipl_path)
      close_rom_file(ddipl);

    if (ddrom_path)
      close_rom_file(ddrom);

    close_rom_file(pifrom);
    return 4;
  }

  return 0;
}

