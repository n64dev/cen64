/* ============================================================================
 *  CEN64.c: Main application.
 *
 *  CEN64: Cycle-accurate, Efficient Nintendo 64 Simulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "CEN64.h"
#include "Device.h"

#ifdef __cplusplus
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#else
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifdef GLFW3
#include <GLFW/glfw3.h>
#else
#include <GL/glfw.h>
#endif

#ifdef GLFW3
GLFWwindow *window;
#endif

/* GLFW seems to like global state. */
/* We'll jmp back into main at close. */
static jmp_buf env;

/* ============================================================================
 *  CloseRequested: GLFW requested a close; jump to saved environment.
 * ========================================================================= */
static int
CloseRequested(void) {
  longjmp(env, 1);
  return 0;
}

/* ============================================================================
 *  WindowResizeCallback: GLFW window resized; fill window, but maintain
 *  aspect ratio.
 * ========================================================================= */
#ifdef GLFW3
static void
WindowResizeCallback(GLFWwindow *window, int width, int height) {
#else
static void
WindowResizeCallback(int width, int height) {
#endif
  float aspect = 4.0 / 3.0;

  if (height <= 0)
    height = 1;

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if((float)width / (float)height > aspect) {
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


/* ============================================================================
 *  ParseArgs: Parses the argument list and performs actions.
 * ========================================================================= */
static void
ParseArgs(int args, const char *argv[], struct CEN64Device *device) {
  int i;

  /* TODO: getops or something sensible. */
  for (i = 0; i < args; i++) {
    const char *arg = argv[i];

    while (*arg == ' ');

    /* Accept -, --, and / */
    if (*arg == '-') {
      arg++;

      if (*arg == '-')
        arg++;
    }

    else if (*arg == '/')
      arg++;

    /* Set backing EEPROM file. */
    if (!strcmp(arg, "eeprom")) {
      if (++i >= args) {
        printf("-eeprom: Missing argument; ignoring.\n");
        continue;
      }

      SetEEPROMFile(device->pif, argv[i]);
    }

    /* Set backing SRAM file. */
    if (!strcmp(arg, "sram")) {
      if (++i >= args) {
        printf("-sram: Missing argument; ignoring.\n");
        continue;
      }

      SetSRAMFile(device->rom, argv[i]);
    }
  }
}

/* ============================================================================
 *  main: Parses arguments and kicks off the application.
 * ========================================================================= */
int main(int argc, const char *argv[]) {
  struct CEN64Device *device;

  if (argc < 3) {
    printf(
      "Usage: %s [options] <pifrom> <cart>\n\n"
      "Options:\n"
      "  -eeprom <file>\n"
      "  -sram <file>\n\n",
      argv[0]);

    printf("RSP Build Type: %s\nRDP Build Type: %s\n",
      RSPBuildType, RDPBuildType);

    return 0;
  }

  if (glfwInit() != GL_TRUE) {
    printf("Failed to initialize GLFW.\n");
    return 255;
  }

#ifdef GLFW3
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  glfwWindowHint(GLFW_RED_BITS, 5);
  glfwWindowHint(GLFW_GREEN_BITS, 6);
  glfwWindowHint(GLFW_BLUE_BITS, 5);
  glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 8);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);

  if ((window = glfwCreateWindow(640, 480, "CEN64", NULL, NULL)) == NULL) {
    debug("Failed to open a GLFW window.");
    glfwTerminate();
    return 0;
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowCloseCallback(window, CloseRequested);
  glfwSetWindowSizeCallback(window, WindowResizeCallback);
#else
  glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_FALSE);
  if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 8, 0, GLFW_WINDOW) != GL_TRUE) {
    printf("Failed to open a GLFW window.\n");

    glfwTerminate();
    return 0;
  }

  glfwSetWindowTitle("CEN64");
  glfwSetWindowCloseCallback(CloseRequested);
  glfwSetWindowSizeCallback(WindowResizeCallback);
#endif
  glfwPollEvents();

  if ((device = CreateDevice(argv[argc - 2])) == NULL) {
    printf("Failed to create a device.\n");

#ifdef GLFW3
    glfwDestroyWindow(window);
    return 1;
  }

  SetVIFContext(device->vif, window);
#else
    glfwCloseWindow();
    return 1;
  }
#endif

  if (LoadCartridge(device, argv[argc - 1])) {
    printf("Failed to load the ROM.\n");

    DestroyDevice(device);
    return 2;
  }

  /* Parse the argument list now that */
  /* the console is ready for us. */
  ParseArgs(argc - 3, argv + 1, device);

  debug("== Booting the Console ==");

  if (setjmp(env) == 0) {
    while (1)
      CycleDevice(device);
  }

  debug("== Destroying the Console ==");

  DestroyDevice(device);
#ifdef GLFW3
  glfwDestroyWindow(window);
#else
  glfwCloseWindow();
#endif
  glfwTerminate();
  return 0;
}

