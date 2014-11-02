//
// os/windows/main.c
//
// Entry point for CEN64.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"
#include "device.h"
#include "options.h"
#include "os/gl_window.h"
#include "os/main.h"
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>

static int cen64_win32_main(int argc, const char *argv[]);
static int load_roms(const char *pifrom_path, const char *cart_path,
  struct rom_file *pifrom, struct rom_file *cart);

static void hide_console(void);
static void show_console(void);

// Windows application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  WSADATA wsa_data = {0};
  WPARAM status = 255;

  if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    MessageBox(NULL, "Failed to initialize Winsock.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return status;
  }

  status = cen64_win32_main(__argc, __argv);

  WSACleanup();
  return status;
}

// Called when another simulation instance is desired.
int cen64_win32_main(int argc, const char *argv[]) {
	struct cen64_options options = default_cen64_options;
  struct rom_file pifrom, cart;
  int status;

  if (argc < 3) {
    show_console();
    print_command_line_usage(argv[0]);
    hide_console();

    return EXIT_SUCCESS;
  }

  if (parse_options(&options, argc - 1, argv + 1)) {
    show_console();

    printf("Invalid command line argument(s) specified.\n");
    print_command_line_usage(argv[0]);

    hide_console();
    return EXIT_FAILURE;
  }

  if (load_roms(options.pifrom_path, options.cart_path, &pifrom, &cart))
    return EXIT_FAILURE;

  status = os_main(&options, &pifrom, &cart);

  close_rom_file(&cart);
  close_rom_file(&pifrom);
  return status;
}

// "Hides" the console window (after waiting for input).
void hide_console(void) {
  printf("\n");
  system("PAUSE");

  FreeConsole();
}

// Load any ROM images required for simulation.
int load_roms(const char *pifrom_path, const char *cart_path,
  struct rom_file *pifrom, struct rom_file *cart) {
  if (open_rom_file(pifrom_path, pifrom)) {
    MessageBox(NULL, "Failed to load PIF ROM.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return 1;
  }

  if (open_rom_file(cart_path, cart)) {
    MessageBox(NULL, "Failed to load cart.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    close_rom_file(pifrom);
    return 2;
  }

  return 0;
}

// Allocates memory for a new device, runs it.
int os_main(struct cen64_options *options,
  struct rom_file *pifrom, struct rom_file *cart) {
  struct gl_window_hints hints;
  int status;

  // Allocate the device on the stack.
  struct cen64_device device;
  memset(&device, 0, sizeof(device));

  // Prevent debugging tools from raising warnings
  // about uninitialized memory being read, etc.
  get_default_gl_window_hints(&hints);

  if (create_gl_window(bus, &device.vi.gl_window, &hints)) {
    MessageBox(NULL, "Failed to create a window.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return 1;
  }

  status = device_run(&device, options, malloc(DEVICE_RAMSIZE), pifrom, cart);
  destroy_gl_window(&device.vi.gl_window);

  return status;
}

// Temporary stub functions.
bool os_exit_requested(struct gl_window *gl_window) { return false; }
void os_render_frame(struct gl_window *gl_window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type) {
}

// "Unhides" the console window.
void show_console(void) {
  AllocConsole();

  freopen("CONOUT$", "wb", stdout);
  freopen("CONOUT$", "wb", stderr);
}

