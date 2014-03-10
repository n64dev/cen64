//
// bus/address.h: System bus address ranges.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __bus_address_h__
#define __bus_address_h__

/* Peripheral interface RAM. */
#define PIF_RAM_BASE_ADDRESS      0x1FC007C0
#define PIF_RAM_ADDRESS_LEN       0x00000040

/* Peripheral interface ROM. */
#define PIF_ROM_BASE_ADDRESS      0x1FC00000
#define PIF_ROM_ADDRESS_LEN       0x000007C0

#endif

