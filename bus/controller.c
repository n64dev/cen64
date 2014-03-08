//
// bus/controller.c: System bus controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/controller.h"
#include "pif/controller.h"

// Initializes the bus component.
int bus_init(struct bus_controller *bus) {
  bus->num_requests = 0;
  bus->rq_head = 0;
  bus->rq_tail = 0;

  return 0;
}

// Issues a read request to the bus.
unsigned bus_read_word(struct bus_controller *bus,
  uint32_t address, uint32_t *word) {

  if (address >= 0x1FC00000 && address < 0x1FC07C00)
    return read_pifrom(bus->pif, word, address & 0xFFC);

  printf("bus_read_word: Failed to access: 0x%.8X\n", address);
  abort();

  return 0;
}

