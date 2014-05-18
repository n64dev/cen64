//
// cen64.c: CEN64 entry point.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device.h"

static void window_resize_cb(int width, int height) {
  float aspect = 4.0 / 3.0;

  if (height <= 0)
    height = 1;

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if((float) width / (float) height > aspect) {
    aspect = 3.0 / 4.0;
    aspect *= (float)width / (float)height;
    glOrtho(-aspect, aspect, -1, 1, -1, 1);
  }

  else {
    aspect *= (float)height / (float)width;
    glOrtho(-1, 1, -aspect, aspect, -1, 1);
  }

  glClear(GL_COLOR_BUFFER_BIT);
}

int cen64_main(int argc, const char *argv[]) {
  struct cen64_device device;
  struct gl_window_hints hints;

  get_default_gl_window_hints(&hints);

  hints.width = 640;
  hints.height = 480;
  hints.fullscreen = 0;

  if (argc < 3) {
    printf("%s <pifrom.bin> <rom>\n", argv[0]);
    return 0;
  }

  if (create_gl_window("CEN64", &device.vi.gl_window, &hints)) {
    printf("Failed to create a window.\n");
    return 1;
  }

  if (device_create(&device, argv[1], argv[2]) == NULL) {
    printf("Failed to create a device.\n");
    return 2;
  }

  device_run(&device);

  device_destroy(&device);
  destroy_gl_window(&device.vi.gl_window);
  return 0;
}

