//
// os/input.c
//
// Input device handling functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/input.h"

void keyboard_press_callback(struct bus_controller *bus, unsigned key) {
  //fprintf(stderr, "os/input: Got keypress event: %u\n", key);
}

void keyboard_release_callback(struct bus_controller *bus, unsigned key) {
  //fprintf(stderr, "os/input: Got keyrelease event: %u\n", key);
}

