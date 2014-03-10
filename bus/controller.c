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
#include "bus/address.h"
#include "bus/controller.h"
#include "bus/memorymap.h"
#include "pif/controller.h"

// Initializes the bus component.
int bus_init(struct bus_controller *bus) {
  if ((bus->map = create_memory_map(1)) == NULL)
    return -1;

  map_address_range(bus->map, PIF_ROM_BASE_ADDRESS, PIF_ROM_ADDRESS_LEN,
    bus->pif, read_pif_rom, write_pif_rom);

  return 0;
}

// Issues a read request to the bus.
int bus_read_word(struct bus_controller *bus,
  uint32_t address, uint32_t *word) {
  const struct memory_mapping *node;

  if ((node = resolve_mapped_address(bus->map, address)) == NULL) {
    fprintf(stderr, "bus_read_word: Failed to access: 0x%.8X\n", address);
    abort();
  }

  return node->on_read(node->instance, address, word);
}

