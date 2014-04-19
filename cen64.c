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
#include <GL/glfw.h>

static int create_glfw_window() {
  glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

  if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 8, 0, GLFW_WINDOW) != GL_TRUE)
    return -2;

  glfwSetWindowTitle("CEN64 [ALPHA]");
  glfwPollEvents();
  return 0;
}

int main(int argc, const char *argv[]) {
  struct cen64_device device;

  if (argc < 3) {
    printf("%s <pifrom.bin> <rom>\n", argv[0]);
    return 0;
  }

  if (device_create(&device, argv[1], argv[2]) == NULL) {
    printf("Failed to create a device.\n");
    return 1;
  }

  if (glfwInit() != GL_TRUE) {
    printf("Failed to initialize GLFW.\n");
    return 2;
  }

  if (create_glfw_window() < 0) {
    printf("Failed to create a window.\n");
    return 3;
  }

  device_run(&device);

  device_destroy(&device);
  glfwTerminate();
  return 0;
}

