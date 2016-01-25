//
// si/pak.h: Controller pak handling
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Mike Ryan.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __si_pak_h__
#define __si_pak_h__
#include "common.h"
#include "os/common/rom_file.h"
#include "os/common/save_file.h"

#define MEMPAK_SIZE 0x8000

enum pak_type {
  PAK_NONE = 0,
  PAK_MEM,
  PAK_RUMBLE,
  PAK_TRANSFER,
};

struct controller {
  const char *mempak_path;
  struct save_file mempak_save;

  const char *tpak_rom_path;
  struct rom_file tpak_rom;
  const char *tpak_save_path;
  struct save_file tpak_save;
  int tpak_mode;
  int tpak_mode_changed;
  int tpak_bank;

  enum pak_type pak;
  int pak_enabled;
  int present;
};

void controller_pak_format(uint8_t *ptr);

int controller_pak_read(struct controller *controller,
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes);
int controller_pak_write(struct controller *controller,
    uint8_t *send_buf, uint8_t send_bytes,
    uint8_t *recv_buf, uint8_t recv_bytes);

#endif
