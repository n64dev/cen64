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
#include "device/sha1.h"
#include "device/sha1_sums.h"
#include "os/common/alloc.h"
#include "os/common/rom_file.h"
#include "os/common/save_file.h"
#include "os/cpuid.h"
#include "pi/is_viewer.h"
#include "thread.h"
#include <stdlib.h>

cen64_cold static int check_extensions(void);
cen64_cold static int load_roms(const char *ddipl_path, const char *ddrom_path,
  const char *pifrom_path, const char *cart_path, struct rom_file *ddipl,
  const struct dd_variant **dd_variant,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart);
cen64_cold static int load_paks(struct controller *controller);
cen64_cold static int validate_sha(struct rom_file *rom, const uint8_t *good_sum);

cen64_cold static int run_device(struct cen64_device *device, bool no_video);
cen64_cold static CEN64_THREAD_RETURN_TYPE run_device_thread(void *opaque);

// Called when another simulation instance is desired.
int cen64_main(int argc, const char **argv) {
  struct controller controller[4] = { { 0, }, };
	struct cen64_options options = default_cen64_options;
  options.controller = controller;
  struct rom_file ddipl, ddrom, pifrom, cart;
  const struct dd_variant *dd_variant;
  struct cen64_mem cen64_device_mem;
  struct cen64_device *device;
  int status;

  const struct cart_db_entry *cart_info;
  struct save_file eeprom;
  struct save_file sram;
  struct save_file flashram;
  struct is_viewer is, *is_in = NULL;

  if (!cart_db_is_well_formed()) {
    printf("Internal cart detection database is not well-formed.\n");
    return EXIT_FAILURE;
  }

  if (cen64_alloc_init()) {
    printf("Failed to initialize the low-level allocators.\n");
    return EXIT_FAILURE;
  }

  if (check_extensions()) {
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
  memset(&flashram, 0, sizeof(flashram));
  memset(&is, 0, sizeof(is));
  dd_variant = NULL;

  if (load_roms(options.ddipl_path, options.ddrom_path, options.pifrom_path,
    options.cart_path, &ddipl, &dd_variant, &ddrom, &pifrom, &cart)) {
    cen64_alloc_cleanup();
    return EXIT_FAILURE;
  }

  if (cart.size >= 0x40 && (cart_info = cart_db_get_entry(cart.ptr)) != NULL) {
    printf("Detected cart: %s[%s] - %s\n", cart_info->rom_id, cart_info->regions, cart_info->description);
    switch (cart_info->save_type) {
      case CART_DB_SAVE_TYPE_EEPROM_4KBIT:
        if (options.eeprom_path == NULL) {
          printf("Warning: cart saves to 4kbit EEPROM, but none specified (see -eep4k)\n");
          open_save_file(NULL, 0x200, &eeprom, NULL);
        } else {
          if (options.eeprom_size != 0x200)
             printf("Warning: cart saves to 4kbit EEPROM, but different size specified (see -eep4k)\n");
        }
        break;
      case CART_DB_SAVE_TYPE_EEPROM_16KBIT:
        if (options.eeprom_path == NULL) {
          printf("Warning: cart saves to 16kbit EEPROM, but none specified (see -eep16k)\n");
          open_save_file(NULL, 0x800, &eeprom, NULL);
        } else {
          if (options.eeprom_size != 0x800)
             printf("Warning: cart saves to 16kbit EEPROM, but different size specified (see -eep16k)\n");
        }
        break;
      case CART_DB_SAVE_TYPE_FLASH_1MBIT:
        if (options.flashram_path == NULL) {
          int created;
          printf("Warning: cart saves to Flash, but none specified (see -flash)\n");
          open_save_file(NULL, FLASHRAM_SIZE, &flashram, &created);
          if (created) {
            memset(flashram.ptr, 0xFF, FLASHRAM_SIZE);
          }
        }
        break;
      case CART_DB_SAVE_TYPE_SRAM_256KBIT:
        if (options.sram_path == NULL) {
          printf("Warning: cart saves to SRAM, but none specified (see -sram)\n");
          open_save_file(NULL, 0x8000, &sram, NULL);
        }
        break;
    }
  }

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

  if (options.flashram_path != NULL) {
    int created;
    if (open_save_file(options.flashram_path, FLASHRAM_SIZE, &flashram, &created)) {
      cen64_alloc_cleanup();
      return EXIT_FAILURE;
    }
    if (created)
      memset(flashram.ptr, 0xFF, FLASHRAM_SIZE);
  }

  if (options.is_viewer_present) {
    if (!is_viewer_init(&is)) {
      cen64_alloc_cleanup();
      return EXIT_FAILURE;
    } else {
      is_in = &is;
    }
  }

  // Allocate memory for and create the device.
  if (cen64_alloc(&cen64_device_mem, sizeof(*device), false) == NULL) {
    printf("Failed to allocate enough memory for a device.\n");
    status = EXIT_FAILURE;
  }

  else {
    device = (struct cen64_device *) cen64_device_mem.ptr;

    if (device_create(device, &ddipl, dd_variant, &ddrom,
      &pifrom, &cart, &eeprom, &sram,
      &flashram, is_in, controller,
      options.no_audio, options.no_video, options.enable_profiling) == NULL) {
      printf("Failed to create a device.\n");
      status = EXIT_FAILURE;
    }

    else {
      device->multithread = options.multithread;
      status = run_device(device, options.no_video);
      device_destroy(device, options.cart_path);
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


enum cpu_extensions {
    EXT_NONE = 0, EXT_SSE2, EXT_SSE3, EXT_SSSE3, EXT_SSE41, EXT_AVX
};

static const char *_cpu_extensions_str(enum cpu_extensions ext) {
    switch (ext) {
        case EXT_NONE:
            return "None";
        case EXT_SSE2:
            return "SSE2";
        case EXT_SSE3:
            return "SSE3";
        case EXT_SSSE3:
            return "SSSE3";
        case EXT_SSE41:
            return "SSE4.1";
        case EXT_AVX:
            return "AVX";
    }
    return "Unknown";
}

// check compiled CPU extensions vs what's supported by running CPU
// returns 0 if the CPU supports the compiled extensions, 1 if not
int check_extensions(void) {
    struct cen64_cpuid_t cpuid;
    enum cpu_extensions max_supported = EXT_NONE, compiled = EXT_NONE;

    // get feature bits
    cen64_cpuid(1, 0, &cpuid);

    if (cpuid.edx & (1 << 26))
        max_supported = EXT_SSE2;
    if (cpuid.ecx & (1 << 0))
        max_supported = EXT_SSE3;
    if (cpuid.ecx & (1 << 9))
        max_supported = EXT_SSSE3;
    if (cpuid.ecx & (1 << 19))
        max_supported = EXT_SSE41;
    if (cpuid.ecx & (1 << 28))
        max_supported = EXT_AVX;

#ifdef __SSE2__
    compiled = EXT_SSE2;
#endif
#ifdef __SSE3__
    compiled = EXT_SSE3;
#endif
#ifdef __SSSE3__
    compiled = EXT_SSSE3;
#endif
#ifdef __SSE4_1__
    compiled = EXT_SSE41;
#endif
#ifdef __AVX__
    compiled = EXT_AVX;
#endif

    if (compiled > max_supported) {
        printf("Error: cen64 is compiled with extensions not supported by your CPU.\n");
        printf("cen64 will not run until you recompile using older extensions.\n");

        printf("\n");
        printf("cen64 compiled with:  %s\n", _cpu_extensions_str(compiled));
        printf("Your CPU supports:    %s\n", _cpu_extensions_str(max_supported));

        return 1;
    }

    if (compiled < max_supported) {
        printf("Warning: cen64 is not using the fastest extensions supported by your CPU.\n");
        printf("cen64 will run, but you can get better performance by recompiling.\n");
        printf("\n");
        printf("cen64 compiled with:  %s\n", _cpu_extensions_str(compiled));
        printf("Your CPU supports:    %s\n", _cpu_extensions_str(max_supported));
    }

    return 0;
}

// Load any ROM images required for simulation.
int load_roms(const char *ddipl_path, const char *ddrom_path,
  const char *pifrom_path, const char *cart_path, struct rom_file *ddipl,
  const struct dd_variant **dd_variant,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart) {
  memset(ddipl, 0, sizeof(*ddipl));

  if (ddipl_path && open_rom_file(ddipl_path, ddipl)) {
    printf("Failed to load DD IPL ROM: %s.\n", ddipl_path);

    return 1;
  }

  *dd_variant = NULL;
  if (ddipl_path != NULL) {
    *dd_variant = dd_identify_variant(ddipl);
    if (*dd_variant != NULL)
      printf("DD variant: %s\n", (*dd_variant)->description);
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

  if (validate_sha(pifrom, sha1_pifrom_ntsc))
    printf("Using NTSC-U PIFROM\n");
  else if (validate_sha(pifrom, sha1_pifrom_ntsc_j))
    printf("Using NTSC-J PIFROM\n");
  else if (validate_sha(pifrom, sha1_pifrom_pal))
    printf("Using PAL PIFROM\n");
  else {
    printf("Unknown or corrupted PIFROM: %s.\n", pifrom_path);

#if 0
    if (ddipl_path)
      close_rom_file(ddipl);

    if (ddrom_path)
      close_rom_file(ddrom);

    return 5;
#endif
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

      gb_init(&controller[i]);
    }
  }
  return 0;
}

int validate_sha(struct rom_file *rom, const uint8_t *good_sum) {
  uint8_t sha1_calc[20];
  sha1(rom->ptr, rom->size, sha1_calc);
  return memcmp(sha1_calc, good_sum, SHA1_SIZE) == 0;
}

// Spins the device until an exit request is received.
int run_device(struct cen64_device *device, bool no_video) {
  cen64_thread thread;

  device->running = true;

  if (cen64_thread_create(&thread, run_device_thread, device)) {
    printf("Failed to create the main emulation thread.\n");
    device_destroy(device, NULL);
    return 1;
  }

  cen64_thread_setname(thread, "device");

  if (no_video)
    pause();
  else
    cen64_gl_window_thread(device);

  device->running = false;
  cen64_thread_join(&thread);
  return 0;
}

CEN64_THREAD_RETURN_TYPE run_device_thread(void *opaque) {
  cen64_thread_setname(NULL, "device");
  struct cen64_device *device = (struct cen64_device *) opaque;

  device_run(device);
  return CEN64_THREAD_RETURN_VAL;
}

