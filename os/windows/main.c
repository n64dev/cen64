//
// os/windows/main.c
//
// Entry point for CEN64.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "cen64.h"
#include "device/device.h"
#include "device/options.h"
#include "os/gl_window.h"
#include "os/main.h"
#include "os/windows/winapi_window.h"
#include <signal.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>

static int cen64_win32_main(int argc, const char *argv[]);
static int load_roms(const char *ddipl_path, const char *ddrom_path,
  const char *pifrom_path, const char *cart_path, struct rom_file *ddipl,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart);

static void hide_console(void);
static void show_console(void);

cen64_cold static DWORD run_device_thread(void *opaque);

HANDLE dynarec_heap;

// Only used when passed -nointerface.
bool device_exit_requested;

cen64_cold static void device_sigint(int signum) {
  device_exit_requested = true;
}

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

  if ((dynarec_heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0)) == NULL) {
    MessageBox(NULL, "Failed to create the dynarec heap.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    WSACleanup();
    return EXIT_FAILURE;
  }

  status = cen64_win32_main(__argc, __argv);
  HeapDestroy(dynarec_heap);
  WSACleanup();

  return status;
}

// Called when another simulation instance is desired.
int cen64_win32_main(int argc, const char *argv[]) {
  struct cen64_options options = default_cen64_options;
  struct rom_file ddipl, ddrom, pifrom, cart;
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

// "Hides" the console window (after waiting for input).
void hide_console(void) {
  printf("\n");
  system("PAUSE");

  FreeConsole();
}

// Load any ROM images required for simulation.
int load_roms(const char *ddipl_path, const char *ddrom_path,
  const char *pifrom_path, const char *cart_path, struct rom_file *ddipl,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart) {
  memset(ddipl, 0, sizeof(*ddipl));

  if (ddipl_path && open_rom_file(ddipl_path, ddipl)) {
    MessageBox(NULL, "Failed to load DD IPL ROM.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    return 1;
  }

  if (ddrom_path && open_rom_file(ddrom_path, ddrom)) {
    MessageBox(NULL, "Failed to load DD ROM.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    if (ddipl_path)
      close_rom_file(ddipl);

    return 2;
  }

  if (open_rom_file(pifrom_path, pifrom)) {
    MessageBox(NULL, "Failed to load PIF ROM.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    if (ddipl_path)
      close_rom_file(ddipl);

    if (ddrom_path)
      close_rom_file(ddrom);

    return 3;
  }

  if (open_rom_file(cart_path, cart)) {
    MessageBox(NULL, "Failed to load cart.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

    if (ddipl_path)
      close_rom_file(ddipl);

    if (ddrom_path)
      close_rom_file(ddrom);

    close_rom_file(pifrom);
    return 4;
  }

  return 0;
}

// Grabs the input lock.
void os_acquire_input(struct gl_window *gl_window) {
  struct winapi_window *winapi_window =
    (struct winapi_window *) (gl_window->window);

  EnterCriticalSection(&winapi_window->event_lock);
}

// Releases the input lock.
void os_release_input(struct gl_window *gl_window) {
  struct winapi_window *winapi_window =
    (struct winapi_window *) (gl_window->window);

  LeaveCriticalSection(&winapi_window->event_lock);
}

// Allocates memory for a new device, runs it.
int os_main(struct cen64_options *options, struct rom_file *ddipl,
  struct rom_file *ddrom, struct rom_file *pifrom, struct rom_file *cart) {
  struct gl_window_hints hints;
  struct winapi_window window;
  int status;

  // Event/rendering thread.
  HANDLE t_hnd;

  // Allocate the device on the stack.
  struct cen64_device device;

  // Prevent debugging tools from raising warnings
  // about uninitialized memory being read, etc.
  memset(&device, 0, sizeof(device));

  if (device_create(&device, malloc(DEVICE_RAMSIZE),
    ddipl, ddrom, pifrom, cart) == NULL) {
    printf("Failed to create a device.\n");

    //deallocate_ram(&hunk);
    return 1;
  }

  // Spawn the user interface (or signal handler).
  if (!options->no_interface) {
    device.vi.gl_window.window = &window;
    get_default_gl_window_hints(&hints);

    if (create_gl_window(&device.bus, &device.vi.gl_window, &hints)) {
      MessageBox(NULL, "Failed to create a window.", "CEN64",
        MB_OK | MB_ICONEXCLAMATION);

      //free(ram);
      return 1;
    }
  }

  else {
    if (signal(SIGINT, device_sigint) == SIG_ERR)
      MessageBox(NULL, "Failed to register SIGINT handler.", "CEN64",
        MB_OK | MB_ICONEXCLAMATION);
  }

  // Start the device thread, hand over control to the UI thread on success.
  if (options->console)
    show_console();

  if ((t_hnd = CreateThread(NULL, 0,
    run_device_thread, &device, 0, NULL)) != NULL) {

    gl_window_thread(&device.vi.gl_window, &device.bus);
    WaitForSingleObject(t_hnd, INFINITE);
  }

  else
    MessageBox(NULL, "Unable to spawn a thread for the device.", "CEN64",
      MB_OK | MB_ICONEXCLAMATION);

  if (options->console)
    hide_console();

  if (!options->no_interface)
    destroy_gl_window(&device.vi.gl_window);

    return status;
}

bool os_exit_requested(struct gl_window *gl_window) {
  struct winapi_window *winapi_window =
    (struct winapi_window *) (gl_window->window);

  return winapi_window_exit_requested(winapi_window);
}

void os_render_frame(struct gl_window *gl_window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type) {
  struct winapi_window *winapi_window =
    (struct winapi_window *) (gl_window->window);

  winapi_window_render_frame(winapi_window, data,
    xres, yres, xskip, type);

  ReleaseSemaphore(winapi_window->render_semaphore, 1, 0);
}

// "Unhides" the console window.
void show_console(void) {
  AllocConsole();

  freopen("CONOUT$", "wb", stdout);
  freopen("CONOUT$", "wb", stderr);
}

// Runs the device, always returns 0.
DWORD run_device_thread(void *opaque) {
  struct cen64_device *device = (struct cen64_device *) opaque;

  device_run(device);
  return 0;
}

