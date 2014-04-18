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

// Audio interface registers.
#define AI_REGS_BASE_ADDRESS      0x04500000
#define AI_REGS_ADDRESS_LEN       0x00000018

// Display processor registers.
#define DP_REGS_BASE_ADDRESS      0x04100000
#define DP_REGS_ADDRESS_LEN       0x00000020

// Parallel interface regs.
#define PI_REGS_BASE_ADDRESS      0x04600000
#define PI_REGS_ADDRESS_LEN       0x00100000

// Peripheral interface RAM.
#define PIF_RAM_BASE_ADDRESS      0x1FC007C0
#define PIF_RAM_ADDRESS_LEN       0x00000040

// Peripheral interface ROM.
#define PIF_ROM_BASE_ADDRESS      0x1FC00000
#define PIF_ROM_ADDRESS_LEN       0x000007C0

// RDRAM Registers.
#define RDRAM_REGS_BASE_ADDRESS   0x03F00000
#define RDRAM_REGS_ADDRESS_LEN    0x00000028

// RAM interface registers.
#define RI_REGS_BASE_ADDRESS      0x04700000
#define RI_REGS_ADDRESS_LEN       0x00000020

// Cartridge ROM.
#define ROM_CART_BASE_ADDRESS     0x10000000
#define ROM_CART_ADDRESS_LEN      0x0FC00000

// Serial interface registers.
#define SI_REGS_BASE_ADDRESS      0x04800000
#define SI_REGS_ADDRESS_LEN       0x0000001C

// SP data/instruction memory.
#define SP_MEM_BASE_ADDRESS       0x04000000
#define SP_MEM_ADDRESS_LEN        0x00002000

// SP interface registers.
#define SP_REGS_BASE_ADDRESS      0x04040000
#define SP_REGS_ADDRESS_LEN       0x00000020

// SP interface registers [2].
#define SP_REGS2_BASE_ADDRESS     0x04080000
#define SP_REGS2_ADDRESS_LEN      0x00000008

// Video interface registers.
#define VI_REGS_BASE_ADDRESS      0x04400000
#define VI_REGS_ADDRESS_LEN       0x00000038

#endif

