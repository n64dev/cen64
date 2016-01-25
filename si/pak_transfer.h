//
// si/pak_transfer.h: Transfer pak handling
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Mike Ryan.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __si_pak_transfer_h__
#define __si_pak_transfer_h__
#include "common.h"

void transfer_pak_read(struct controller *controller,
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes);
void transfer_pak_write(struct controller *controller,
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes);

#endif
