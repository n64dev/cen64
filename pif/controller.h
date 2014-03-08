//
// pif/controller.h: Peripheral interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __pif_controller_h__
#define __pif_controller_h__
#include "common.h"

struct bus_controller *bus;

struct pif_controller {
  struct bus_controller *bus;
  const uint8_t *rom;
};

int init_pif(struct pif_controller *pif,
  struct bus_controller *bus, const uint8_t *rom);

int read_pifrom(struct pif_controller *pif, uint32_t *word, unsigned off);

#endif

