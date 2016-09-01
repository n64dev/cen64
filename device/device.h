//
// device.h: Common CEN64 device container.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __device_h__
#define __device_h__
#include "common.h"
#include "device/options.h"
#include "os/common/rom_file.h"
#include "os/common/save_file.h"

#include "ai/controller.h"
#include "bus/controller.h"
#include "dd/controller.h"
#include "pi/controller.h"
#include "ri/controller.h"
#include "si/controller.h"
#include "rdp/cpu.h"
#include "rsp/cpu.h"
#include "thread.h"
#include "vi/controller.h"
#include "vr4300/cpu.h"

// Only used when passed -nointerface.
extern bool device_exit_requested;

struct cen64_device {
  struct bus_controller bus;
  struct vr4300 vr4300;

  struct ai_controller ai;
  struct dd_controller dd;
  struct pi_controller pi;
  struct ri_controller ri;
  struct si_controller si;
  struct vi_controller vi;

  struct rdp rdp;
  struct rsp rsp;

  int debug_sfd;

  bool multithread;
  bool other_thread_is_waiting;

  cen64_thread device_thread;
  cen64_thread os_thread;
  cen64_thread vr4300_thread;

  cen64_mutex sync_mutex;
  cen64_cv sync_cv;

  bool running;
};

cen64_cold void device_destroy(struct cen64_device *device);
cen64_cold struct cen64_device *device_create(struct cen64_device *device,
  const struct rom_file *ddipl, const struct dd_variant *dd_variant,
  const struct rom_file *ddrom,
  const struct rom_file *pifrom, const struct rom_file *cart,
  const struct save_file *eeprom, const struct save_file *sram,
  const struct save_file *flashram, const struct controller *controller,
  bool no_audio, bool no_video);

cen64_cold void device_exit(struct bus_controller *bus);
cen64_cold void device_run(struct cen64_device *device);

#endif

