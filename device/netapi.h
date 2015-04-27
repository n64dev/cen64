//
// device/netapi.h: CEN64 device network API.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __device_netapi_h__
#define __device_netapi_h__
#include "common.h"

struct device;

enum netapi_debug_request_type {
  NETAPI_DEBUG_ERROR,
  NETAPI_DEBUG_GET_PROTOCOL_VERSION,
  NETAPI_DEBUG_GET_VR4300_REGS,
};

struct netapi_debug_request {
  uint32_t magic;
  uint32_t seq_id;
  uint32_t length;

  enum netapi_debug_request_type type;
  uint8_t data[];
};

cen64_cold void netapi_close_connection(int csfd);
cen64_cold int netapi_open_connection(void);

cen64_cold int netapi_debug_wait(int sfd, struct cen64_device *device);

#endif

