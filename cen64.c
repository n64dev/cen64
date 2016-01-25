//
// cen64.c: CEN64 entry point.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/controller.h"
#include "cen64.h"
#include "device/cart_db.h"
#include "device/device.h"
#include "device/options.h"
#include "os/common/alloc.h"
#include "os/common/rom_file.h"
#include "os/common/save_file.h"
#include "thread.h"
#include <stdlib.h>

cen64_cold static int load_roms(const char *ddipl_path, const char *ddrom_path,
  const char *pifrom_path, const char *cart_path, struct rom_file *ddipl,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart);
cen64_cold static int load_paks(struct controller *controller);

cen64_cold static int run_device(struct cen64_device *device, bool no_video);
cen64_cold static CEN64_THREAD_RETURN_TYPE run_device_thread(void *opaque);

// Called when another simulation instance is desired.
int cen64_main(int argc, const char **argv) {
  struct controller controller[4] = { { NULL, }, };
	struct cen64_options options = default_cen64_options;
  options.controller = controller;
  struct rom_file ddipl, ddrom, pifrom, cart;
  struct cen64_mem cen64_device_mem;
  struct cen64_device *device;
  int status;

  const struct cart_db_entry *cart_info;
  struct save_file eeprom;
  struct save_file sram;

  if (cart_db_is_well_formed()) {
    printf("Internal cart detection database is not well-formed.\n");
    return EXIT_FAILURE;
  }

  if (cen64_alloc_init()) {
    printf("Failed to initialize the low-level allocators.\n");
    return EXIT_FAILURE;
  }

  if (argc < 3) {
    print_command_line_usage(argv[0]);
    cen64_alloc_cleanup();
    return EXIT_SUCCESS;
  }

  if (parse_options(&options, argc - 1, argv + 1)) {
    printf("Invalid command line argument(s) specified.\n");

    print_command_line_usage(argv[0]);
    cen64_alloc_cleanup();
    return EXIT_FAILURE;
  }

  memset(&ddipl, 0, sizeof(ddipl));
  memset(&ddrom, 0, sizeof(ddrom));
  memset(&cart,  0, sizeof(cart));
  memset(&eeprom, 0, sizeof(eeprom));
  memset(&sram,  0, sizeof(sram));

  if (load_roms(options.ddipl_path, options.ddrom_path, options.pifrom_path,
    options.cart_path, &ddipl, &ddrom, &pifrom, &cart)) {
    cen64_alloc_cleanup();
    return EXIT_FAILURE;
  }

  if (cart.size >= 0x40 && (cart_info = cart_db_get_entry(cart.ptr)) != NULL)
    printf("Detected cart: %s\n", cart_info->image_name);

  if (load_paks(controller)) {
    cen64_alloc_cleanup();
    return EXIT_FAILURE;
  }

  if (options.eeprom_path != NULL &&
      open_save_file(options.eeprom_path, options.eeprom_size, &eeprom, NULL)) {
    cen64_alloc_cleanup();
    return EXIT_FAILURE;
  }

  if (options.sram_path != NULL &&
      open_save_file(options.sram_path, 0x8000, &sram, NULL)) {
    cen64_alloc_cleanup();
    return EXIT_FAILURE;
  }

  // Allocate memory for and create the device.
  if (cen64_alloc(&cen64_device_mem, sizeof(*device), false) == NULL) {
    printf("Failed to allocate enough memory for a device.\n");
    status = EXIT_FAILURE;
  }

  else {
    device = (struct cen64_device *) cen64_device_mem.ptr;

    if (device_create(device, &ddipl, &ddrom, &pifrom, &cart, &eeprom, &sram,
      controller, options.no_audio, options.no_video) == NULL) {
      printf("Failed to create a device.\n");
      status = EXIT_FAILURE;
    }

    else {
      device->multithread = options.multithread;
      status = run_device(device, options.no_video);
      device_destroy(device);
    }

    cen64_free(&cen64_device_mem);
  }

  // Release resources.
  if (options.ddipl_path)
    close_rom_file(&ddipl);

  if (options.ddrom_path)
    close_rom_file(&ddrom);

  if (options.cart_path)
    close_rom_file(&cart);

  close_rom_file(&pifrom);
  cen64_alloc_cleanup();
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

int load_paks(struct controller *controller) {
  int i;

  for (i = 0; i < 4; ++i) {
    if (controller[i].pak == PAK_MEM && controller[i].mempak_path != NULL) {
      int created = 0;
      if (open_save_file(controller[i].mempak_path, MEMPAK_SIZE, &controller[i].mempak_save, &created) != 0) {
        printf("Can't open mempak file %s\n", controller[i].mempak_path);
        return -1;
      }
      if (created)
        controller_pak_format(controller[i].mempak_save.ptr);
    }

    else if (controller[i].pak == PAK_TRANSFER) {
      if (controller[i].tpak_rom_path != NULL) {
        if (open_rom_file(controller[i].tpak_rom_path,
              &controller[i].tpak_rom)) {
          printf("Can't open transfer pak ROM\n");
          return -1;
        }
      } else {
        printf("No ROM supplied for transfer pak.\n");
        printf("The game will run but probably won't do anything interest\n");
      }
      if (controller[i].tpak_save_path != NULL) {
        if (open_gb_save(controller[i].tpak_save_path,
              &controller[i].tpak_save)) {
          printf("Can't open transfer pak save\n");
          return -1;
        }
      } else {
        printf("No save supplied for transfer pak. Just FYI.\n");
      }

    }
  }
  return 0;
}

// Spins the device until an exit request is received.
int run_device(struct cen64_device *device, bool no_video) {
  cen64_thread thread;

  if (cen64_thread_create(&thread, run_device_thread, device)) {
    printf("Failed to create the main emulation thread.\n");
    device_destroy(device);
    return 1;
  }

  if (!no_video)
    cen64_gl_window_thread(device);

  cen64_thread_join(&thread);
  return 0;
}

CEN64_THREAD_RETURN_TYPE run_device_thread(void *opaque) {
  struct cen64_device *device = (struct cen64_device *) opaque;

  device_run(device);
  return CEN64_THREAD_RETURN_VAL;
}

