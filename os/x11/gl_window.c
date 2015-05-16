//
// os/x11/gl_window.c: X11/OpenGL window definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device/device.h"
#include "gl_common.h"
#include "gl_config.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "gl_window.h"
#include "input.h"
#include "timer.h"
#include "vi/controller.h"
#include "vi/render.h"
#include <unistd.h>
#include <X11/Xlib.h>

static int cen64_gl_window_create_objects(cen64_gl_window window);
static bool cen64_gl_window_pump_events(struct vi_controller *vi);

// Creates an (initially hidden) cen64_gl_window.
cen64_gl_window cen64_gl_window_create(
  cen64_gl_display display, cen64_gl_screen screen,
  const cen64_gl_config *config, const char *title) {
  cen64_gl_window window;

  if ((window = malloc(sizeof(*window))) == NULL)
    return CEN64_GL_WINDOW_BAD;

  // Get the visual info for the framebuffer configuration.
  if ((window->visual_info = glXGetVisualFromFBConfig(
    display, *config)) == NULL) {
    close(window->pipefds[0]);
    close(window->pipefds[1]);

    cen64_mutex_destroy(&window->event_mutex);
    free(window);

    return CEN64_GL_WINDOW_BAD;
  }

  // Create synchronization primitives for the window.
  if (cen64_gl_window_create_objects(window)) {
    XFree(window->visual_info);
    free(window);

    return CEN64_GL_WINDOW_BAD;
  }

  // Create a colormap using the visual info.
  window->attr.colormap = XCreateColormap(display, XRootWindow(
    display, screen), window->visual_info->visual, AllocNone);

  // Select the events we'd like to receive, create the window.
  window->attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    ButtonPressMask | StructureNotifyMask;

  window->window = XCreateWindow(display, XRootWindow(display, screen),
    0, 0, 640, 480, 0, window->visual_info->depth, InputOutput,
    window->visual_info->visual, CWBorderPixel | CWColormap | CWEventMask,
    &window->attr);

  // Now that we created the window, setup any atoms/properties we need.
  XSetStandardProperties(display, window->window, title,
    NULL, None, NULL, 0, NULL);

  window->wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window->window, &window->wm_delete_window, 1);

  window->exit_requested = false;
  window->display = display;
  window->screen = screen;
  return window;
}

// Handles events that come from X11.
bool cen64_gl_window_pump_events(struct vi_controller *vi) {
  bool released, exit_requested = false;
  XEvent event;

  if (!XPending(vi->display))
    return false;

  cen64_mutex_lock(&vi->window->event_mutex);

  do {
    XNextEvent(vi->display, &event);

    switch (event.type) {
      case ClientMessage:
        vi->window->exit_requested = exit_requested = true;
        break;

      case ConfigureNotify:
        gl_window_resize_cb(event.xconfigure.width, event.xconfigure.height);
        break;

      case KeyPress:
        keyboard_press_callback(vi->bus, XLookupKeysym(&event.xkey, 0));
        break;

      case KeyRelease:
        released = true;

        // Detect and correct auto-repeated keys. Auto-repeated KeyEvents
        // will be inserted immediately after the release.
        if (XEventsQueued(vi->display, QueuedAfterReading)) {
          XEvent next_event;

          XPeekEvent(vi->display, &next_event);
          if (next_event.type == KeyPress && next_event.xkey.time ==
            event.xkey.time && next_event.xkey.keycode == event.xkey.keycode) {
            XNextEvent(vi->display, &event);
            released = false;
          }
        }

        if (released)
          keyboard_release_callback(vi->bus, XLookupKeysym(&event.xkey, 0));

        break;
    }
  } while (XPending(vi->display));

  cen64_mutex_unlock(&vi->window->event_mutex);

  return exit_requested;
}

// Allocate mutexes, pipes, etc. for the UI/window.
int cen64_gl_window_create_objects(cen64_gl_window window) {
  if (cen64_mutex_create(&window->event_mutex)) {
    return 1;
  }

  if (cen64_mutex_create(&window->render_mutex)) {
    cen64_mutex_destroy(&window->event_mutex);
    return 1;
  }

  if (pipe(window->pipefds) < 0) {
    cen64_mutex_destroy(&window->render_mutex);
    cen64_mutex_destroy(&window->event_mutex);
    return 1;
  }

  return 0;
}

// Thread that controls the user interface, etc.
int cen64_gl_window_thread(struct cen64_device *device) {
  struct vi_controller *vi = &device->vi;
  cen64_time last_update_time;
  cen64_gl_window window;
  unsigned frame_count;

  int max_fds, x11_fd;
  fd_set fdset;

  // We listen for UI updates using a POSIX pipe, and X11
  // events/messages using the connection fd. select() is
  // then used to efficiently multiplex between the two,
  // so setup a fd_set, pipes, etc. required for all this.
  x11_fd = ConnectionNumber(vi->display);

  max_fds = x11_fd > vi->window->pipefds[0] ? x11_fd: vi->window->pipefds[0];
  max_fds++;

  FD_ZERO(&fdset);
  FD_SET(vi->window->pipefds[0], &fdset);
  FD_SET(x11_fd, &fdset);

  // Okay, main UI thread loop starts here.
  for (frame_count = 0, get_time(&last_update_time) ; ;) {
    fd_set ready_to_read = fdset;

    // Multiplex between all of our event sources...
    if (select(max_fds, &ready_to_read, NULL, NULL, NULL) > 0) {

      // Did we get a X11 event?
      //if (FD_ISSET(x11_fd, &ready_to_read)) {
        if (unlikely(cen64_gl_window_pump_events(vi)))
          break;
      //}

      // Did we get a UI event?
      if (FD_ISSET(vi->window->pipefds[0], &ready_to_read)) {
        read(vi->window->pipefds[0], &window, sizeof(window));

        cen64_mutex_lock(&window->render_mutex);

        gl_window_render_frame(vi, window->frame_buffer,
          window->frame_hres, window->frame_vres,
          window->frame_hskip, window->frame_type);

        cen64_mutex_unlock(&window->render_mutex);

        // Update the window title every 60 VIs
        // to display the current VI/s rate.
        if (++frame_count == 60) {
          char title[128];
          cen64_time current_time;
          float ns;

          // Compute time spent rendering last 60 frames, reset timer/counter.
          get_time(&current_time);
          ns = compute_time_difference(&current_time, &last_update_time);
          last_update_time = current_time;
          frame_count = 0;

          sprintf(title,
            "CEN64 ["CEN64_COMPILER" - "CEN64_ARCH_DIR"/"CEN64_ARCH_SUPPORT"]"
            " - %.1f VI/s", (60 / (ns / NS_PER_SEC)));

          cen64_gl_window_set_title(window, title);
        }
      }
    }
  }

  return 0;
}

