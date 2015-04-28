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
#include "gl_common.h"
#include "gl_config.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "vi/render.h"
#include <stddef.h>
#include <stdlib.h>
#include <X11/Xlib.h>

bool cen64_gl_window_pump_events(struct vi_controller *vi) {
  bool released, exit_requested;
  XEvent event;

  if (!XPending(vi->display))
    return false;

  exit_requested = false;
  //pthread_mutex_lock(&glx_window->event_lock);

  do {
    XNextEvent(vi->display, &event);

    switch (event.type) {
      case ClientMessage:
        //if ((unsigned) event.xclient.data.l[0] == glx_window->wm_delete_message)
        //  glx_window->exit_requested = exit_requested = true;

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

  //pthread_mutex_unlock(&glx_window->event_lock);
  return exit_requested;
}

