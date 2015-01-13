//
// os/unix/x11/glx_window.c
//
// Convenience functions for managing rendering windows.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "bus/controller.h"
#include "common.h"
#include "common/debug.h"
#include "os/gl_window.h"
#include "os/input.h"
#include "os/timer.h"
#include "os/unix/x11/glx_window.h"
#include "vi/controller.h"

#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>

// Functions to assist in taming X11.
cen64_cold static int create_glx_context(
  struct glx_window *glx_window, GLXContext *context);

cen64_cold static int destroy_glx_window(struct glx_window *glx_window);

cen64_cold static void generate_attribute_list(int *attribute_list,
  const struct gl_window_hints *hints);

cen64_cold static int get_matching_visual_info(struct glx_window *glx_window,
  int *attribute_list, XVisualInfo **visual_info);

cen64_cold static int get_matching_window_mode(struct glx_window *glx_window,
  const struct gl_window_hints *hints, XF86VidModeModeInfo *mode);

cen64_cold static bool glx_window_poll_events(struct bus_controller *bus,
  struct glx_window *glx_window);

cen64_cold static int glx_window_thread(struct gl_window *gl_window,
  struct glx_window *glx_window, struct bus_controller *bus);

cen64_cold static void glx_window_update_window_title(
  struct glx_window *glx_window, cen64_time *last_report_time);

cen64_cold static int switch_to_fullscreen(struct glx_window *glx_window,
  const struct gl_window_hints *hints);

// Creates a new rendering context.
int create_glx_context(struct glx_window *glx_window, GLXContext *context) {
  GLXContext check_context = glXCreateContext(glx_window->display,
    glx_window->visual_info, 0, GL_TRUE);

  if (check_context == NULL) {
    debug("create_glx_context: Unknown client-side error.\n");
    return 1;
  }

  else if (check_context == (void*) BadAlloc) {
    debug("create_glx_context: Server is out of resources.\n");
    return 1;
  }

  else if (check_context == (void*) BadMatch) {
    debug("create_glx_context: Context address space mismatch.\n");
    return 1;
  }

  else if (check_context == (void*) BadValue) {
    debug("create_glx_context: Unsupported visual mode specified.\n");
    return 1;
  }

  *context = check_context;
  return 0;
}

// Jumps to the entry point for the user interface code.
int gl_window_thread(struct gl_window *gl_window, struct bus_controller *bus) {
  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);
  return glx_window_thread(gl_window, glx_window, bus);
}

// Creates a new rendering window.
int create_gl_window(struct bus_controller *bus,
  struct gl_window *gl_window, const struct gl_window_hints *hints) {
  struct glx_window *glx_window;
  int window_valuemask;
  Window root_window;

  // We may set this to false if we fail
  // to change active the windowing mode.
  int fullscreen = hints->fullscreen;

  // Magic number was chosen based on the glXChooseFBConfig man page.
  // It is at least large enough to hold the supported attributes, as
  // well as a few additional ones. Expand it at your convenience.
  int attribute_list[64];

  debug("create_gl_window: Creating window...\n");
  glx_window = (struct glx_window *) (gl_window->window);
  memset(glx_window, 0, sizeof(*glx_window));

  pthread_mutex_init(&glx_window->event_lock, NULL);
  pthread_mutex_init(&glx_window->render_lock, NULL);
  pthread_cond_init(&glx_window->render_cv, NULL);

  // Open a connection and get the default screen number.
  if ((glx_window->display = XOpenDisplay(NULL)) == NULL) {
    debug("create_gl_window: Could not open connection to server.\n");
    return 1;
  }

  glx_window->screen = DefaultScreen(glx_window->display);
  root_window = RootWindow(glx_window->display, glx_window->screen);

  // Use hints to create a window, then bind the GL context.
  generate_attribute_list(attribute_list, hints);

  if (get_matching_visual_info(glx_window,
    attribute_list, &glx_window->visual_info)) {
    debug("create_gl_window: Failed to match window hints.\n");
    goto create_out_destroy;
  }

  if (create_glx_context(glx_window, &glx_window->context)) {
    debug("create_gl_window: Failed to acquire a GL context.\n");
    goto create_out_destroy;
  }

  glx_window->attr.event_mask = ExposureMask | KeyPressMask |
    KeyReleaseMask | ButtonPressMask | StructureNotifyMask;

  if (!(glx_window->attr.colormap = XCreateColormap(glx_window->display,
    root_window, glx_window->visual_info->visual, AllocNone))) {
    debug("create_gl_window: Failed to create a colormap.\n");
    goto create_out_destroy;
  }

  window_valuemask = CWBorderPixel | CWColormap | CWEventMask;

  // If going fullscreen, tell XF86 to set the mode.
  if (fullscreen) {
    if (!switch_to_fullscreen(glx_window, hints)) {
      glx_window->went_fullscreen = 1;
      glx_window->attr.override_redirect = True;
      window_valuemask |= CWOverrideRedirect;
    }

    else {
      debug("create_gl_window: Failed to to go fullscreen; falling back.\n");
      fullscreen = 0;
    }
  }

  if (!(glx_window->window = XCreateWindow(glx_window->display, root_window,
    0, 0, hints->width, hints->height, 0, glx_window->visual_info->depth,
    InputOutput, glx_window->visual_info->visual, window_valuemask,
    &glx_window->attr))) {
    debug("create_gl_window: Failed to create a window.\n");
    goto create_out_destroy;
  }

  glx_window->wm_delete_message = XInternAtom(
    glx_window->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(glx_window->display, glx_window->window,
    &glx_window->wm_delete_message, 1);

  // If going fullscreen, hide the pointer, trap input devices, etc.
  if (fullscreen) {
    XWarpPointer(glx_window->display, None,
      glx_window->window, 0, 0, 0, 0, 0, 0);

    XGrabKeyboard(glx_window->display, glx_window->window, True,
      GrabModeAsync, GrabModeAsync, CurrentTime);

    XGrabPointer(glx_window->display, glx_window->window, True,
      ButtonPressMask, GrabModeAsync, GrabModeAsync,
      glx_window->window, None, CurrentTime);
  }

  XSetStandardProperties(glx_window->display, glx_window->window,
    "CEN64 ["CEN64_COMPILER" - "CEN64_ARCH_DIR"/"CEN64_ARCH_SUPPORT"]",
    "CEN64", None, NULL, 0, NULL);

  XMapRaised(glx_window->display, glx_window->window);
  return 0;

create_out_destroy:
  destroy_glx_window(glx_window);
  return 1;
}

// Destroys an existing rendering window.
int destroy_gl_window(struct gl_window *gl_window) {
  struct glx_window *glx_window = (struct glx_window *) (gl_window->window);

  // Wait for the window to tear down, first.
  pthread_join(glx_window->thread, NULL);

  pthread_mutex_lock(&glx_window->event_lock);
  pthread_mutex_lock(&glx_window->render_lock);
  destroy_glx_window(glx_window);

  pthread_cond_destroy(&glx_window->render_cv);
  pthread_mutex_unlock(&glx_window->render_lock);
  pthread_mutex_destroy(&glx_window->render_lock);
  pthread_mutex_unlock(&glx_window->event_lock);
  pthread_mutex_destroy(&glx_window->event_lock);

  return 0;
}

// Releases resources acquired for a rendering window.
int destroy_glx_window(struct glx_window *glx_window) {
  if (glx_window->went_fullscreen) {
    XF86VidModeSwitchToMode(glx_window->display,
      glx_window->screen, &glx_window->old_mode);
  }

  if (glx_window->context) {
    if (!glXMakeCurrent(glx_window->display, None, NULL)) {
      debug("destroy_glx_window: Could not release rendering context.\n");
      return 1;
    }

    glXDestroyContext(glx_window->display, glx_window->context);
    glx_window->context = NULL;
  }

  if (glx_window->window) {
    XDestroyWindow(glx_window->display, glx_window->window);
    glx_window->window = 0;
  }

  if (glx_window->attr.colormap) {
    XFreeColormap(glx_window->display, glx_window->attr.colormap);
    glx_window->attr.colormap = 0;
  }

  if (glx_window->visual_info) {
    XFree(glx_window->visual_info);
    glx_window->visual_info = NULL;
  }

  if (glx_window->display) {
    XCloseDisplay(glx_window->display);
    glx_window->display = NULL;
  }

  return 0;
}

// Fills the array with attributes that best match the hints.
void generate_attribute_list(int *attribute_list,
  const struct gl_window_hints *hints) {
  int idx = 0;

  attribute_list[idx++] = GLX_DRAWABLE_TYPE;
  attribute_list[idx++] = GLX_WINDOW_BIT;
  attribute_list[idx++] = GLX_RENDER_TYPE;
  attribute_list[idx++] = GLX_RGBA_BIT;

  attribute_list[idx++] = GLX_DOUBLEBUFFER;
  attribute_list[idx++] = hints->double_buffered
    ? True : False;

  attribute_list[idx++] = GLX_RED_SIZE;
  attribute_list[idx++] = hints->color_bits;
  attribute_list[idx++] = GLX_GREEN_SIZE;
  attribute_list[idx++] = hints->color_bits;
  attribute_list[idx++] = GLX_BLUE_SIZE;
  attribute_list[idx++] = hints->color_bits;
  attribute_list[idx++] = GLX_ALPHA_SIZE;
  attribute_list[idx++] = hints->alpha_bits;

  attribute_list[idx++] = GLX_DEPTH_SIZE;
  attribute_list[idx++] = hints->depth_bits;

  attribute_list[idx++] = GLX_STENCIL_SIZE;
  attribute_list[idx++] = hints->stencil_bits;

  attribute_list[idx++] = GLX_ACCUM_RED_SIZE;
  attribute_list[idx++] = hints->accum_color_bits;
  attribute_list[idx++] = GLX_ACCUM_GREEN_SIZE;
  attribute_list[idx++] = hints->accum_color_bits;
  attribute_list[idx++] = GLX_ACCUM_BLUE_SIZE;
  attribute_list[idx++] = hints->accum_color_bits;
  attribute_list[idx++] = GLX_ACCUM_ALPHA_SIZE;
  attribute_list[idx++] = hints->accum_alpha_bits;

  attribute_list[idx++] = GLX_AUX_BUFFERS;
  attribute_list[idx++] = hints->auxiliary_buffers;

  /* Terminate the list. */
  attribute_list[idx++] = None;
}

// Packs hints with a reasonable set of default hints.
void get_default_gl_window_hints(struct gl_window_hints *hints) {
  memset(hints, 0, sizeof(*hints));

  hints->width = 640;
  hints->height = 480;

  hints->fullscreen = 0;
  hints->double_buffered = 1;
}

// Generates a XVisualInfo that matches the attributes.
int get_matching_visual_info(struct glx_window *window,
  int *attribute_list, XVisualInfo **visual_info) {
  XVisualInfo *check_visual_info = NULL;
  int i, status = 0, num_configs = 0;
  GLXFBConfig *fb_configs;

  if ((fb_configs = glXChooseFBConfig(window->display, window->screen,
    attribute_list, &num_configs)) == NULL || num_configs == 0) {
    debug("get_matching_visual_info: No matching framebuffer configs.\n");
    return 1;
  }

  for (i = 0; i < num_configs && check_visual_info == NULL; i++)
    check_visual_info = glXGetVisualFromFBConfig(window->display, *fb_configs);

  if (check_visual_info == NULL) {
    debug("get_matching_visual_info: Could not get associated visual info.\n");
    status = 1;
  }

  *visual_info = check_visual_info;
  XFree(fb_configs);
  return status;
}

// Finds a windowing most at least as big as desired resolution.
int get_matching_window_mode(struct glx_window *glx_window,
  const struct gl_window_hints *hints, XF86VidModeModeInfo *mode) {
  XF86VidModeModeInfo **modes;
  int i, not_found, num_modes;

  not_found = 1;
  num_modes = 0;

  if (!XF86VidModeGetAllModeLines(glx_window->display,
    glx_window->screen, &num_modes, &modes) || num_modes <= 0) {
    debug("get_matching_window_mode: Failed to query window modes.\n");
    return 1;
  }

  *mode = *modes[0];

  for (i = 0; i < num_modes - 1; i++) {
    unsigned width = modes[i]->hdisplay;
    unsigned height = modes[i]->vdisplay;

    if (width == hints->width && height == hints->height) {
      *mode = *modes[i];
      not_found = 0;
      break;
    }
  }

  XFree(modes);
  return not_found;
}

// Promotes the contents of the back buffer to the front buffer.
int gl_swap_buffers(const struct gl_window *window) {
  const struct glx_window *glx_window;

  glx_window = (const struct glx_window *) (window->window);
  glXSwapBuffers(glx_window->display, glx_window->window);
  return 0;
}

// Informs the caller if an exit was requested.
bool glx_window_exit_requested(struct glx_window *window) {
  bool exit_requested;

  pthread_mutex_lock(&window->event_lock);
  exit_requested = window->exit_requested;
  pthread_mutex_unlock(&window->event_lock);

  return exit_requested;
}

// Handles events that come from X11.
bool glx_window_poll_events(struct bus_controller *bus,
  struct glx_window *glx_window) {
  bool released, exit_requested;
  XEvent event;

  exit_requested = false;

  if (!XPending(glx_window->display))
    return false;

  pthread_mutex_lock(&glx_window->event_lock);

  do {
    XNextEvent(glx_window->display, &event);

    switch (event.type) {
      case ClientMessage:
        if ((unsigned) event.xclient.data.l[0] == glx_window->wm_delete_message)
          glx_window->exit_requested = exit_requested = true;

        break;

      case ConfigureNotify:
        gl_window_resize_cb(event.xconfigure.width, event.xconfigure.height);
        break;

      case KeyPress:
        keyboard_press_callback(bus, XLookupKeysym(&event.xkey, 0));
        break;

      case KeyRelease:
        released = true;

        // Detect and correct auto-repeated keys. Auto-repeated KeyEvents
        // will be inserted immediately after the release.
        if (XEventsQueued(glx_window->display, QueuedAfterReading)) {
          XEvent next_event;

          XPeekEvent(glx_window->display, &next_event);
          if (next_event.type == KeyPress && next_event.xkey.time ==
            event.xkey.time && next_event.xkey.keycode == event.xkey.keycode) {
            XNextEvent(glx_window->display, &event);
            released = false;
          }
        }

        if (released)
          keyboard_release_callback(bus, XLookupKeysym(&event.xkey, 0));

        break;
    }
  } while (XPending(glx_window->display));

  pthread_mutex_unlock(&glx_window->event_lock);
  return exit_requested;
}

// Copies the frame data to the render thread.
void glx_window_render_frame(struct glx_window *window, const void *data,
  unsigned xres, unsigned yres, unsigned xskip, unsigned type) {
  size_t copy_size;

  switch (type & 0x3) {
    case 0:
    case 1:
      copy_size = 0;
      break;

		// TODO: Why 4?
    case 2:
      copy_size = 4;
      break;

    case 3:
      copy_size = 4;
      break;
  }

  copy_size *= xres * yres;

  // Grab the locks and dump the data.
  // TODO: Check to make sure !frame_pending.
  pthread_mutex_lock(&window->render_lock);

  memcpy(window->frame_data, data, copy_size);
  window->frame_xres = xres;
  window->frame_yres = yres;
  window->frame_xskip = xskip;
  window->frame_type = type;
  window->frame_pending = true;

  pthread_mutex_unlock(&window->render_lock);
}

// Main window threads. Handles and pumps events.
int glx_window_thread(struct gl_window *gl_window,
  struct glx_window *glx_window, struct bus_controller *bus) {
  cen64_time last_report_time;
  cen64_time refresh_timeout;
  unsigned frame_count = 0;

  pthread_mutex_lock(&glx_window->render_lock);

  // Activate the rendering context from THIS thread.
  // Any kind of GL call has to be done from here, or else.
  if (!glXMakeCurrent(glx_window->display,
    glx_window->window, glx_window->context)) {
    debug("glx_window_thread: Could not attach rendering context.\n");

    return 1;
  }

  gl_window_init(gl_window);
  pthread_mutex_unlock(&glx_window->render_lock);

  get_time(&last_report_time);

  // Poll for input periodically (~1000x a second) while checking
  // to see if the simulator pushed out any frames in that time.
  while (1) {
    if (unlikely(frame_count == 60)) {
      glx_window_update_window_title(glx_window, &last_report_time);
      frame_count = 0;
    }

    if (glx_window_poll_events(bus, glx_window))
      break;

    get_time(&refresh_timeout);
#ifdef __APPLE__
    refresh_timeout.tv_usec += 1000LL;

    if (refresh_timeout.tv_usec >= 1000000LL) {
      refresh_timeout.tv_usec -= 1000000LL;
#else
    refresh_timeout.tv_nsec += 1000000LL;

    if (refresh_timeout.tv_nsec >= 1000000000LL) {
      refresh_timeout.tv_nsec -= 1000000000LL;
#endif
      refresh_timeout.tv_sec += 1;
    }

    // Check if we were signaled. If not, just sit around for now.
    pthread_mutex_lock(&glx_window->render_lock);

    if (glx_window->frame_pending || pthread_cond_wait(
      &glx_window->render_cv, &glx_window->render_lock) != ETIMEDOUT) {
      glx_window->frame_pending = false;
      frame_count++;

      gl_window_render_frame(gl_window, glx_window->frame_data,
        glx_window->frame_xres, glx_window->frame_yres,
        glx_window->frame_xskip, glx_window->frame_type);
    }

    pthread_mutex_unlock(&glx_window->render_lock);
  }

  return 0;
}

// Updates the window title.
void glx_window_update_window_title(
  struct glx_window *glx_window, cen64_time *last_report_time) {
  cen64_time current_time;
  unsigned long long ns;
  char window_title[64];

  get_time(&current_time);
  ns = compute_time_difference(&current_time, last_report_time);

  snprintf(window_title, sizeof(window_title),
    "CEN64 ["CEN64_COMPILER" - "CEN64_ARCH_DIR"/"CEN64_ARCH_SUPPORT"]"
    " - %.1f VI/s", (60 / ((double) ns / NS_PER_SEC)));

  XStoreName(glx_window->display, glx_window->window, window_title);
  *last_report_time = current_time;
}

// Attempts to switch to fullscreen mode.
int switch_to_fullscreen(struct glx_window *glx_window,
  const struct gl_window_hints *hints) {
  XF86VidModeModeInfo mode, **modes;
  int num_modes;

  // Get the current mode (so we can switch back later).
  if (!XF86VidModeGetAllModeLines(glx_window->display,
    glx_window->screen, &num_modes, &modes)) {
    debug("switch_to_fullscreen: Failed to query current mode.\n");
    return 1;
  }

  glx_window->old_mode = *modes[0];
  XFree(modes);

  // Try to switch the mode, possibly falling back to windowed mode.
  if (get_matching_window_mode(glx_window, hints, &mode) ||
    !XF86VidModeSwitchToMode(glx_window->display, glx_window->screen, &mode)) {
    return 1;
  }

  XF86VidModeSetViewPort(glx_window->display, glx_window->screen, 0, 0);
  return 0;
}

