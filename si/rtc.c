//
// si/rtc.c: RTC routines
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Mike Ryan.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "local_time.h"

int rtc_status(uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes) {
  recv_buf[0] = 0x00;
  recv_buf[1] = 0x10;
  recv_buf[2] = 0x00;

  return 0;
}

int rtc_read(uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes) {
  struct time_stamp now;

  // FIXME is this needed?
  memset(recv_buf, 0, recv_bytes);

  // read RTC block
  switch (send_buf[1]) {
    case 0:
      recv_buf[0] = 0x02;
      break;

    case 1:
      debug("RTC cannot read block 1\n");
      return 1;

    case 2:
      get_local_time(&now);
      recv_buf[0] = byte2bcd(now.sec);
      recv_buf[1] = byte2bcd(now.min);
      recv_buf[2] = 0x80 + byte2bcd(now.hour);
      recv_buf[3] = byte2bcd(now.day);
      recv_buf[4] = byte2bcd(now.week_day);
      recv_buf[5] = byte2bcd(now.month);
      recv_buf[6] = byte2bcd(now.year);
      recv_buf[7] = byte2bcd(now.year / 100);
      recv_buf[8] = 0x00; // status
      break;

    default:
      debug("RTC unknown block\n");
      return 1;
  }

  return 0;
}

int rtc_write(uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes) {
  debug("RTC write not implemented\n");
  return 1;
}
