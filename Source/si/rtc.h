//
// si/rtc.h: RTC routines
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Mike Ryan.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __si_rtc_h__
#define __si_rtc_h__
#include "common.h"
#include "si/controller.h"

void rtc_init(struct rtc * rtc);
int rtc_status(struct rtc * rtc, 
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes);
int rtc_read(struct rtc * rtc, 
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes);
int rtc_write(struct rtc * rtc,
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes);

#endif
