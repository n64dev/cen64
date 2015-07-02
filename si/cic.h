//
// si/cic.h: PIF CIC security/lock out algorithms.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __si_cic_h__
#define __si_cic_h__

int get_cic_seed(const uint8_t *rom_data, uint32_t *cic_seed);

#endif

