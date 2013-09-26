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

#ifdef GLFW3
#include <GLFW/glfw3.h>
#endif

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
struct RDP;
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
  struct VIFController *, struct RDP *, struct RSP *,
  struct VR4300 *);

struct VR4300 *CreateVR4300(void);
struct RDP *CreateRDP(void);
struct RSP *CreateRSP(void);

uint32_t GetCICSeed(const struct ROMController *);
int InsertCart(struct ROMController *, const char *);
void SetCICSeed(struct PIFController *, uint32_t);
void SetEEPROMFile(struct PIFController *, const char *);
#ifdef GLFW3
void SetVIFContext(struct VIFController *, GLFWwindow *);
#endif

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
void DestroyRDP(struct RDP *);
void DestroyRSP(struct RSP *);

/* ============================================================================
 *  Component cycle functions.
 * ========================================================================= */
void CycleAIF(struct AIFController *);
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

  struct RDP *rdp;
  struct RSP *rsp;
  struct VR4300 *vr4300;
};

struct CEN64Device *CreateDevice(const char *);
void CycleDevice(struct CEN64Device *);
void DestroyDevice(struct CEN64Device *);
int LoadCartridge(struct CEN64Device *, const char *);

#endif

