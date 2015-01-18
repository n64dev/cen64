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

cen64_cold void netapi_close_connection(int csfd);
cen64_cold int netapi_open_connection(void);

#endif

