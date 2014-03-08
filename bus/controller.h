//
// bus/controller.h: System bus controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __bus_controller_h__
#define __bus_controller_h__
#define BUS_REQUEST_QUEUE_SIZE 8

struct bus_request;
struct vr4300;

struct bus_controller {
  struct bus_request *rq[BUS_REQUEST_QUEUE_SIZE];
  unsigned num_requests, rq_head, rq_tail;

  struct pif_controller *pif;
  struct vr4300 *vr4300;
};

int bus_init(struct bus_controller *bus);
unsigned bus_read_word(struct bus_controller *bus,
  uint32_t address, uint32_t *word);

#endif

