//
// os/x11/gl_window.c: X11/OpenGL window definitions.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device/device.h"
#include "gl_common.h"
#include "gl_display.h"
#include "gl_screen.h"
#include "gl_window.h"
#include "input.h"
#include "timer.h"
#include "vi/controller.h"
#include "vi/render.h"
#include <unistd.h>
#include <SDL.h>

static int cen64_gl_window_create_objects(cen64_gl_window window);
static bool cen64_gl_window_pump_events(struct vi_controller *vi,
  cen64_time *last_update_time, unsigned *frame_count);

// Creates an (initially hidden) cen64_gl_window.
cen64_gl_window cen64_gl_window_create(
    cen64_gl_display display, cen64_gl_screen screen,
    const cen64_gl_config *config, const char *title)
{
    cen64_gl_window window;

    if ((window = malloc(sizeof(*window))) == NULL)
        return CEN64_GL_WINDOW_BAD;

    // Create synchronization primitives for the window.
    if (cen64_gl_window_create_objects(window))
    {
        free(window);
        return CEN64_GL_WINDOW_BAD;
    }
    window->window = SDL_CreateWindow(
        "cen64",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        474,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window->exit_requested = false;
    window->display = display;
    window->screen = screen;
    return window;
}

// Handles events that come from SDL.
bool cen64_gl_window_pump_events(struct vi_controller *vi,
  cen64_time *last_update_time, unsigned *frame_count)
{
    bool released, exit_requested = false;
    SDL_Event e;

    cen64_mutex_lock(&vi->window->event_mutex);

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            vi->window->exit_requested = exit_requested = true;
            break;
        }
        if (e.type == SDL_WINDOWEVENT)
        {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                gl_window_resize_cb(e.window.data1, e.window.data2);
            }
        }
        if (e.type == SDL_KEYDOWN)
        {
            keyboard_press_callback(vi->bus, e.key.keysym.sym);
        }
        if (e.type == SDL_KEYUP)
        {
            keyboard_release_callback(vi->bus, e.key.keysym.sym);
        }
        if (e.type == SDL_USEREVENT)
        {
            cen64_gl_window window = vi->window;

            cen64_mutex_lock(&window->render_mutex);

            gl_window_render_frame(vi, window->frame_buffer,
                                   window->frame_hres, window->frame_vres,
                                   window->frame_hskip, window->frame_type);

            cen64_mutex_unlock(&window->render_mutex);

            // Update the window title every 60 VIs
            // to display the current VI/s rate.
            if (++(*frame_count) == 60)
            {
                char title[128];
                cen64_time current_time;
                float ns;

                // Compute time spent rendering last 60 frames, reset timer/counter.
                get_time(&current_time);
                ns = compute_time_difference(&current_time, last_update_time);
                *last_update_time = current_time;
                *frame_count = 0;

                sprintf(title,
                        "CEN64 [" CEN64_COMPILER " - " CEN64_ARCH_DIR "/" CEN64_ARCH_SUPPORT "]"
                        " - %.1f VI/s",
                        (60 / (ns / NS_PER_SEC)));

                cen64_gl_window_set_title(window, title);
            }
        }
    }
    cen64_mutex_unlock(&vi->window->event_mutex);

    return exit_requested;
}

// Allocate mutexes, pipes, etc. for the UI/window.
int cen64_gl_window_create_objects(cen64_gl_window window)
{
    if (cen64_mutex_create(&window->event_mutex))
    {
        return 1;
    }

    if (cen64_mutex_create(&window->render_mutex))
    {
        cen64_mutex_destroy(&window->event_mutex);
        return 1;
    }

    if (pipe(window->pipefds) < 0)
    {
        cen64_mutex_destroy(&window->render_mutex);
        cen64_mutex_destroy(&window->event_mutex);
        return 1;
    }

    return 0;
}

// Thread that controls the user interface, etc.
int cen64_gl_window_thread(struct cen64_device *device)
{
   cen64_time last_update_time;
  unsigned frame_count;

  get_time(&last_update_time);
  frame_count = 0;

  while (!cen64_gl_window_pump_events(&device->vi,
    &last_update_time, &frame_count));

  return 0;
}
