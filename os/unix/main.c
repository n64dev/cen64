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
#include "device/netapi.h"
#include "device/options.h"
#include "os/common/alloc.h"
#include "os/gl_window.h"
#include "os/main.h"
#include "os/unix/x11/glx_window.h"
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

cen64_cold static void *run_device_thread(void *opaque);

// Only used when passed -nointerface.
bool device_exit_requested;

cen64_cold static void device_sigint(int signum) {
  device_exit_requested = true;
}

// Unix application entry point.
int main(int argc, const char *argv[]) {
  return cen64_main(argc, argv);
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
#if 0
bool os_exit_requested(struct gl_window *gl_window) {
  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);

  return glx_window_exit_requested(glx_window);
}
#endif

// Allocates memory for a new device, runs it.
int os_main(struct cen64_device *device, struct cen64_options *options) {
  struct gl_window_hints hints;
  struct glx_window window;
  pthread_t device_thread;

#if 0
  // Spawn the user interface (or signal handler).
  if (!options->no_interface) {
    device->vi.gl_window.window = &window;
    get_default_gl_window_hints(&hints);

    if (create_gl_window(&device->bus, &device->vi.gl_window, &hints)) {
      printf("Failed to create a window.\n");
      return 1;
    }
  }

  else {
    if (signal(SIGINT, device_sigint) == SIG_ERR)
      printf("Failed to register SIGINT handler.\n");
  }
#endif

  // Pull up the debug API if it was requested.
  device->debug_sfd = -1;

  if (options->enable_debugger) {
    if ((device->debug_sfd = netapi_open_connection()) < 0) {
      printf("Failed to bind/listen for a connection.\n");

      //destroy_gl_window(&device->vi.gl_window);
      device_destroy(device);
      return 1;
    }
  }

  device_run(device);
#if 0
  // Start the device thread, hand over control to the UI thread on success.
  if ((pthread_create(&device_thread, NULL, run_device_thread, device)) == 0) {
    //gl_window_thread(&device->vi.gl_window, &device->bus);
    pthread_join(device_thread, NULL);
  }

  else
    printf("Unable to spawn a thread for the device.\n");
#endif

  if (device->debug_sfd >= 0)
    netapi_close_connection(device->debug_sfd);

#if 0
  if (!options->no_interface)
    destroy_gl_window(&device->vi.gl_window);
#endif

  return 0;
}

// Pushes a frame to the rendering thread.
void os_render_frame(cen64_gl_window window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type) {
//  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);

//  glx_window_render_frame(glx_window, data, xres, yres, xskip, type);
}

// Runs the device, always returns NULL.
void *run_device_thread(void *opaque) {
  struct cen64_device *device = (struct cen64_device *) opaque;

  device_run(device);
  return NULL;
}

