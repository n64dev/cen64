//
// os/input.h
//
// Input device handling functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_input_h__
#define __os_input_h__

struct bus_controller;

void keyboard_press_callback(struct bus_controller *bus, unsigned key);
void keyboard_release_callback(struct bus_controller *bus, unsigned key);

#endif

