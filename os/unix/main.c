//
// os/unix/main.c
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
#include "os/unix/glx_window.h"
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct ram_hunk {
  size_t size;
  void *ptr;
};

cen64_cold static uint8_t *allocate_ram(struct ram_hunk *ram, size_t size);
cen64_cold static void deallocate_ram(struct ram_hunk *ram);

// Global file descriptor for allocations.
#ifdef __linux__
const char *zero_page_path = "/dev/zero";
#else
const char *zero_page_path = "/dev/null";
#endif

int zero_page_fd;

// Allocates a large hunk of zeroed RAM.
uint8_t *allocate_ram(struct ram_hunk *ram, size_t size) {
  if ((ram->ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
    MAP_PRIVATE, zero_page_fd, 0)) == MAP_FAILED)
    return NULL;

#ifndef __linux__
  memset(ram->ptr, 0, size);
#endif
  ram->size = size;
  return ram->ptr;
}

// Allocates a large hunk of RAM.
void deallocate_ram(struct ram_hunk *ram) {
  munmap(ram->ptr, ram->size);
}

// Unix application entry point.
int main(int argc, const char *argv[]) {
  int status;

  if ((zero_page_fd = open(zero_page_path, O_RDWR)) < 0) {
    printf("Failed to open: %s\n", zero_page_path);
    return EXIT_FAILURE;
  }

  status = cen64_cmdline_main(argc, argv);

  close(zero_page_fd);
  return status;
}

// Grabs the input lock.
void os_acquire_input(struct gl_window *gl_window) {
  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);

  pthread_mutex_lock(&glx_window->event_lock);
}

// Releases the input lock.
void os_release_input(struct gl_window *gl_window) {
  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);

  pthread_mutex_unlock(&glx_window->event_lock);
}

// Informs the simulation thread if an exit was requested.
bool os_exit_requested(struct gl_window *gl_window) {
  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);

  return glx_window_exit_requested(glx_window);
}

// Allocates memory for a new device, runs it.
int os_main(struct cen64_options *options,
  struct rom_file *pifrom, struct rom_file *cart) {
  struct gl_window_hints hints;
  struct glx_window window;
  int status;

  // Allocate the device on the stack.
  struct cen64_device device;
  struct ram_hunk hunk;
  uint8_t *ram;

  if ((ram = allocate_ram(&hunk, DEVICE_RAMSIZE)) == NULL) {
    printf("Failed to allocate enough memory.\n");
    return 1;
  }

  memset(&device, 0, sizeof(device));

  // Prevent debugging tools from raising warnings
  // about uninitialized memory being read, etc.
  if (!options->no_interface) {

    device.vi.gl_window.window = &window;
    get_default_gl_window_hints(&hints);

    if (create_gl_window(&device.bus, &device.vi.gl_window, &hints)) {
      printf("Failed to create a window.\n");

      deallocate_ram(&hunk);
      return 1;
    }
  }

  status = device_run(&device, options, ram, pifrom, cart);

  if (!options->no_interface)
    destroy_gl_window(&device.vi.gl_window);

  deallocate_ram(&hunk);
  return status;
}

// Pushes a frame to the rendering thread.
void os_render_frame(struct gl_window *gl_window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type) {
  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);

  glx_window_render_frame(glx_window, data, xres, yres, xskip, type);
  pthread_cond_signal(&glx_window->render_cv);
}

