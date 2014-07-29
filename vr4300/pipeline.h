//
// vr4300/pipeline.h: VR4300 processor pipeline.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __vr4300_pipeline_h__
#define __vr4300_pipeline_h__
#include "common.h"
#include "vr4300/decoder.h"
#include "vr4300/fault.h"
#include "vr4300/segment.h"

struct vr4300;

enum vr4300_bus_request_type {
  VR4300_BUS_REQUEST_NONE,
  VR4300_BUS_REQUEST_READ,
  VR4300_BUS_REQUEST_WRITE,
};

struct vr4300_bus_request {
  uint64_t address;
  uint64_t data;
  uint64_t dqm;
  unsigned size;
  int postshift;
  char two_words;

  enum vr4300_bus_request_type type;
};

struct vr4300_latch {
  uint64_t pc;
  enum vr4300_fault_id fault;
  uint32_t cause_data;
};

struct vr4300_icrf_latch {
  struct vr4300_latch common;
  const struct segment *segment;
  uint64_t pc;
};

struct vr4300_rfex_latch {
  struct vr4300_latch common;
  struct vr4300_opcode opcode;
  uint32_t iw, iw_mask;
};

struct vr4300_exdc_latch {
  struct vr4300_latch common;
  const struct segment *segment;
  int64_t result;
  uint32_t dest;

  struct vr4300_bus_request request;
};

struct vr4300_dcwb_latch {
  struct vr4300_latch common;
  int64_t result;
  uint32_t dest;
};

struct vr4300_pipeline {
  struct vr4300_dcwb_latch dcwb_latch;
  struct vr4300_exdc_latch exdc_latch;
  struct vr4300_rfex_latch rfex_latch;
  struct vr4300_icrf_latch icrf_latch;

  unsigned exception_history;
  unsigned cycles_to_stall;
  bool fault_present;
};

void vr4300_pipeline_init(struct vr4300_pipeline *pipeline);

#endif

