//
// si/pak_transfer.c: Controller pak routines
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Mike Ryan.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "pak.h"
#include "gb.h"

static void gameboy_read(struct controller *controller, uint16_t address,
    uint8_t *buffer);
static void gameboy_write(struct controller *controller, uint16_t address,
    uint8_t *buffer);

void transfer_pak_read(struct controller *controller,
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes) {

  uint16_t address = send_buf[1] << 8 | send_buf[2];
  address &= ~0b11111; // lower 5 bits are address CRC
  // printf("read from %04x\n", address);

  // get enable/disable state
  if (address == 0x8000)
    memset(recv_buf, controller->pak_enabled ? 0x84 : 0x00, 0x20);

  // get insertion status and access mode
  else if (address == 0xB000) {
    if (controller->pak_enabled) {
      // cart inserted, return mode and mode changed
      if (controller->tpak_rom.ptr != NULL) {
        memset(recv_buf, controller->tpak_mode == 1 ? 0x89 : 0x80, 0x20);
        recv_buf[0] |= controller->tpak_mode_changed ? 0b100 : 0;
      }

      // cart not inserted
      else
        memset(recv_buf, 0x40, 0x20);

      controller->tpak_mode_changed = 0;
    }
  }

  // read from GB cart
  else if (address >= 0xC000) {
    if (controller->pak_enabled) {
      uint16_t gb_addr = address - 0xC000
        + (controller->tpak_bank & 3) * 0x4000;
      gameboy_read(controller, gb_addr, recv_buf);
    }
  }
}

void transfer_pak_write(struct controller *controller,
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes) {

  uint16_t address = send_buf[1] << 8 | send_buf[2];
  address &= ~0b11111; // lower 5 bits are address CRC
  // printf("write to %04x\n", address);

  // set bank
  if (address == 0xA000) {
    if (controller->pak_enabled)
      controller->tpak_bank = send_buf[3];
  }

  // set access mode
  else if (address == 0xB000) {
    if (controller->pak_enabled) {
      controller->tpak_mode = send_buf[3] & 1;
      controller->tpak_mode_changed = 1;
    }
  }

  // write to GB cart
  else if (address >= 0xC000) {
    uint16_t gb_addr = address - 0xC000
      + (controller->tpak_bank & 3) * 0x4000;
    gameboy_write(controller, gb_addr, send_buf + 3);
  }
}

// read 0x20 bytes from Game Boy cart at address
void gameboy_read(struct controller *controller, uint16_t address,
    uint8_t *buffer) {
  for(int i=0;i<0x20;i++)
    buffer[i] = gb_read(controller, address+i);
  
//   printf("read:  %04X: ", address);
//   for(int i=0;i<32;i++) {
//     if(i==16) printf("\n             ");
//     printf("%02X ", buffer[i]);
//   }
//   printf("\n");
}

// write 0x20 bytes from buffer to Game Boy cart at address
void gameboy_write(struct controller *controller, uint16_t address,
    uint8_t *buffer) {
  for(int i=0;i<0x20;i++)
    gb_write(controller, address+i, buffer[i]);
//   printf("write: %04X:%02X\n", address, buffer[0]);
}
