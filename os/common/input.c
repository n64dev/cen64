//
// os/common/input.c: Input device handling functions.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "bus/controller.h"
#include "common.h"
#include "input.h"
#include "os/keycodes.h"
#include "si/controller.h"

bool shift_down;
bool left_down;
bool right_down;
bool up_down;
bool down_down;

void keyboard_press_callback(struct bus_controller *bus, unsigned key) {
  struct si_controller *si = bus->si;

  //fprintf(stderr, "os/input: Got keypress event: %u\n", key);

  if (key == CEN64_KEY_LSHIFT || key == CEN64_KEY_RSHIFT) {
    shift_down = true;
    return;
  }

  switch (key) {
    // Analog stick.
    case CEN64_KEY_LEFT:
      si->input[2] = shift_down ? -38 : -114;
      left_down = true;
      break;

    case CEN64_KEY_RIGHT:
      si->input[2] = shift_down ? 38 : 114;
      right_down = true;
      break;

    case CEN64_KEY_UP:
      si->input[3] = shift_down ? 38 : 114;
      up_down = true;
      break;

    case CEN64_KEY_DOWN:
      si->input[3] = shift_down ? -38 : -114;
      down_down = true;
      break;

    // L/R flippers.
    case CEN64_KEY_A: si->input[1] |= 1 << 5; break;
    case CEN64_KEY_S: si->input[1] |= 1 << 4; break;

    // A/Z/B/S buttons.
    case CEN64_KEY_X: si->input[0] |= 1 << 7; break;
    case CEN64_KEY_C: si->input[0] |= 1 << 6; break;
    case CEN64_KEY_Z: si->input[0] |= 1 << 5; break;
    case CEN64_KEY_RETURN: si->input[0] |= 1 << 4; break;

    // D-pad.
    case CEN64_KEY_J: si->input[0] |= 1 << 1; break;
    case CEN64_KEY_L: si->input[0] |= 1 << 0; break;
    case CEN64_KEY_I: si->input[0] |= 1 << 3; break;
    case CEN64_KEY_K: si->input[0] |= 1 << 2; break;

    // C-pad.
    case CEN64_KEY_F: si->input[1] |= 1 << 1; break;
    case CEN64_KEY_H: si->input[1] |= 1 << 0; break;
    case CEN64_KEY_T: si->input[1] |= 1 << 3; break;
    case CEN64_KEY_G: si->input[1] |= 1 << 2; break;
  }
}

void keyboard_release_callback(struct bus_controller *bus, unsigned key) {
  struct si_controller *si = bus->si;

  //fprintf(stderr, "os/input: Got keyrelease event: %u\n", key);

  if (key == CEN64_KEY_LSHIFT || key == CEN64_KEY_RSHIFT) {
    shift_down = false;
    return;
  }

  switch (key) {
    // Analog stick.
    case CEN64_KEY_LEFT:
      si->input[2] = right_down ? (shift_down ? 38 : 114) : 0;
      left_down = false;
      break;

    case CEN64_KEY_RIGHT:
      si->input[2] = left_down ? (shift_down ? -38 : -114) : 0;
      right_down = false;
      break;

    case CEN64_KEY_UP:
      si->input[3] = down_down ? (shift_down ? -38 : -114) : 0;
      up_down = false;
      break;

    case CEN64_KEY_DOWN:
      si->input[3] = up_down ? (shift_down ? 38 : 114) : 0;
      down_down = false;
      break;

    // L/R flippers.
    case CEN64_KEY_A: si->input[1] &= ~(1 << 5); break;
    case CEN64_KEY_S: si->input[1] &= ~(1 << 4); break;

    // A/Z/B/S buttons.
    case CEN64_KEY_X: si->input[0] &= ~(1 << 7); break;
    case CEN64_KEY_C: si->input[0] &= ~(1 << 6); break;
    case CEN64_KEY_Z: si->input[0] &= ~(1 << 5); break;
    case CEN64_KEY_RETURN: si->input[0] &= ~(1 << 4); break;

    // D-pad.
    case CEN64_KEY_J: si->input[0] &= ~(1 << 1); break;
    case CEN64_KEY_L: si->input[0] &= ~(1 << 0); break;
    case CEN64_KEY_I: si->input[0] &= ~(1 << 3); break;
    case CEN64_KEY_K: si->input[0] &= ~(1 << 2); break;

    // C-pad.
    case CEN64_KEY_F: si->input[1] &= ~(1 << 1); break;
    case CEN64_KEY_H: si->input[1] &= ~(1 << 0); break;
    case CEN64_KEY_T: si->input[1] &= ~(1 << 3); break;
    case CEN64_KEY_G: si->input[1] &= ~(1 << 2); break;
  }
}

