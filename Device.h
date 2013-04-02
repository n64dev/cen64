/* ============================================================================
 *  Device.h: Device manager.
 *
 *  CEN64: Cycle-accurate, Efficient Nintendo 64 Simulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __CEN64__DEVICE_H__
#define __CEN64__DEVICE_H__
#include "Common.h"

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

/* ============================================================================
 *  Component structures.
 * ========================================================================= */
struct AIFController;
struct BusController;
struct PIFController;
struct RDRAMController;
struct ROMController;
struct VIFController;

struct VR4300;
struct RSP;

/* ============================================================================
 *  Component create/attach functions.
 * ========================================================================= */
struct AIFController *CreateAIF(void);
struct PIFController *CreatePIF(const char *);
struct RDRAMController *CreateRDRAM(void);
struct ROMController *CreateROM(void);
struct VIFController *CreateVIF(void);

struct BusController *CreateBus(
  struct AIFController *, struct PIFController *,
  struct RDRAMController *, struct ROMController *,
  struct RSP *, struct VIFController *, struct VR4300 *);

struct VR4300 *CreateVR4300(void);
struct RSP *CreateRSP(void);

uint32_t GetCICSeed(const struct ROMController *);
int InsertCart(struct ROMController *, const char *);
void SetCICSeed(struct PIFController *, uint32_t);

/* ============================================================================
 *  Component destroy functions.
 * ========================================================================= */
void DestroyAIF(struct AIFController *);
void DestroyBus(struct BusController *);
void DestroyPIF(struct PIFController *);
void DestroyRDRAM(struct RDRAMController *);
void DestroyROM(struct ROMController *);
void DestroyVIF(struct VIFController *);

void DestroyVR4300(struct VR4300 *);
void DestroyRSP(struct RSP *);

/* ============================================================================
 *  Component cycle functions.
 * ========================================================================= */
void CycleVIF(struct VIFController *);
void CycleVR4300(struct VR4300 *);
void CycleRSP(struct RSP *);


/* ============================================================================
 *  CEN64Device definition.
 * ========================================================================= */
struct CEN64Device {
  struct AIFController *aif;
  struct BusController *bus;
  struct PIFController *pif;
  struct RDRAMController *rdram;
  struct ROMController *rom;
  struct VIFController *vif;

  struct RSP *rsp;
  struct VR4300 *vr4300;
  unsigned long long cycles;
};

struct CEN64Device *CreateDevice(const char *);
void CycleDevice(struct CEN64Device *);
void DestroyDevice(struct CEN64Device *);
int LoadCartridge(struct CEN64Device *, const char *);

#endif

