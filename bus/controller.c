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
#include "ai/controller.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "bus/memorymap.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rdp/cpu.h"
#include "rdp/interface.h"
#include "rsp/cpu.h"
#include "rsp/interface.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"
#include "vr4300/interface.h"

// Initializes the bus component.
int bus_init(struct bus_controller *bus) {
  if ((bus->map = create_memory_map(14)) == NULL)
    return -1;

  map_address_range(bus->map, AI_REGS_BASE_ADDRESS, AI_REGS_ADDRESS_LEN,
    bus->ai, read_ai_regs, write_ai_regs);

  map_address_range(bus->map, DP_REGS_BASE_ADDRESS, DP_REGS_ADDRESS_LEN,
    bus->rdp, read_dp_regs, write_dp_regs);

  map_address_range(bus->map, MI_REGS_BASE_ADDRESS, MI_REGS_ADDRESS_LEN,
    bus->vr4300, read_mi_regs, write_mi_regs);

  map_address_range(bus->map, PI_REGS_BASE_ADDRESS, PI_REGS_ADDRESS_LEN,
    bus->pi, read_pi_regs, write_pi_regs);

  map_address_range(bus->map, PIF_RAM_BASE_ADDRESS, PIF_RAM_ADDRESS_LEN,
    bus->si, read_pif_ram, write_pif_ram);

  map_address_range(bus->map, PIF_ROM_BASE_ADDRESS, PIF_ROM_ADDRESS_LEN,
    bus->si, read_pif_rom, write_pif_rom);

  map_address_range(bus->map, RDRAM_REGS_BASE_ADDRESS, RDRAM_REGS_ADDRESS_LEN,
    bus->ri, read_rdram_regs, write_rdram_regs);

  map_address_range(bus->map, RI_REGS_BASE_ADDRESS, RI_REGS_ADDRESS_LEN,
    bus->ri, read_ri_regs, write_ri_regs);

  map_address_range(bus->map, ROM_CART_BASE_ADDRESS, ROM_CART_ADDRESS_LEN,
    bus->pi, read_cart_rom, write_cart_rom);

  map_address_range(bus->map, SI_REGS_BASE_ADDRESS, SI_REGS_ADDRESS_LEN,
    bus->si, read_si_regs, write_si_regs);

  map_address_range(bus->map, SP_MEM_BASE_ADDRESS, SP_MEM_ADDRESS_LEN,
    bus->rsp, read_sp_mem, write_sp_mem);

  map_address_range(bus->map, SP_REGS_BASE_ADDRESS, SP_REGS_ADDRESS_LEN,
    bus->rsp, read_sp_regs, write_sp_regs);

  map_address_range(bus->map, SP_REGS2_BASE_ADDRESS, SP_REGS2_ADDRESS_LEN,
    bus->rsp, read_sp_regs2, write_sp_regs2);

  map_address_range(bus->map, VI_REGS_BASE_ADDRESS, VI_REGS_ADDRESS_LEN,
    bus->vi, read_vi_regs, write_vi_regs);

  return 0;
}

// Issues a read request to the bus.
int bus_read_word(struct bus_controller *bus,
  uint32_t address, uint32_t *word) {
  const struct memory_mapping *node;

  if ((node = resolve_mapped_address(bus->map, address)) == NULL) {
    fprintf(stderr, "bus_read_word: Failed to access: 0x%.8X\n", address);
    return 0;
  }

  return node->on_read(node->instance, address, word);
}

// Issues a write request to the bus.
int bus_write_word(struct bus_controller *bus,
  uint32_t address, uint32_t word, uint32_t dqm) {
  const struct memory_mapping *node;

  if ((node = resolve_mapped_address(bus->map, address)) == NULL) {
    fprintf(stderr, "bus_write_word: Failed to access: 0x%.8X\n", address);
    return 0;
  }

  return node->on_write(node->instance, address, word, dqm);
}

