//
// bus/address.h: System bus address ranges.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __bus_address_h__
#define __bus_address_h__

// Audio interface registers.
#define AI_REGS_BASE_ADDRESS      0x04500000
#define AI_REGS_ADDRESS_LEN       0x00000018

// DD C2S and DS buffers, DD interface regs, and DD microsequencer RAM.
// These entries are all stuffed into one to reduce memorymap entries.
#define DD_REGS_BASE_ADDRESS      0x05000500
#define DD_MS_RAM_ADDRESS         0x05000580

#define DD_C2S_BUFFER_LEN         0x00000400
#define DD_DS_BUFFER_LEN          0x00000100
#define DD_REGS_ADDRESS_LEN       0x00000080
#define DD_MS_RAM_LEN             0x00000040

#define DD_CONTROLLER_ADDRESS     0x05000000
#define DD_CONTROLLER_LEN         (DD_C2S_BUFFER_LEN   + \
                                   DD_DS_BUFFER_LEN    + \
                                   DD_REGS_ADDRESS_LEN + \
                                   DD_MS_RAM_LEN         \
                                  )

// DD IPL ROM.
#define DD_IPL_ROM_ADDRESS        0x06000000
#define DD_IPL_ROM_LEN            0x00400000

// Display processor registers.
#define DP_REGS_BASE_ADDRESS      0x04100000
#define DP_REGS_ADDRESS_LEN       0x00000020

// FlashRAM registers.
#define FLASHRAM_BASE_ADDRESS     0x08000000
#define FLASHRAM_ADDRESS_LEN      0x00010004 // FIXME is this accurate?

// MIPS interface registers.
#define MI_REGS_BASE_ADDRESS      0x04300000
#define MI_REGS_ADDRESS_LEN       0x00000010

// Parallel interface regs.
#define PI_REGS_BASE_ADDRESS      0x04600000
#define PI_REGS_ADDRESS_LEN       0x00100000

// Peripheral interface RAM and ROM.
#define PIF_ROM_BASE_ADDRESS      0x1FC00000
#define PIF_ROM_ADDRESS_LEN       0x000007C0

#define PIF_RAM_BASE_ADDRESS      0x1FC007C0
#define PIF_RAM_ADDRESS_LEN       0x00000040

#define PIF_BASE_ADDRESS          PIF_ROM_BASE_ADDRESS
#define PIF_ADDRESS_LEN           (PIF_ROM_ADDRESS_LEN + PIF_RAM_ADDRESS_LEN)

// Physical memory.
#define RDRAM_BASE_ADDRESS        0x00000000
#define RDRAM_BASE_ADDRESS_LEN    0x00800000

// RDRAM registers.
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

// SRAM.
#define SRAM_BASE_ADDRESS         0x08000000
#define SRAM_ADDRESS_LEN          0x0801FFFF

// Video interface registers.
#define VI_REGS_BASE_ADDRESS      0x04400000
#define VI_REGS_ADDRESS_LEN       0x00000038

#endif

