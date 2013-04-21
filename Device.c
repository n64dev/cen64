/* ============================================================================
 *  Device.c: Device manager.
 *
 *  CEN64: Cycle-accurate, Efficient Nintendo 64 Simulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Device.h"

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

/* ============================================================================
 *  CreateDevice: Creates a new CEN64 device.
 * ========================================================================= */
struct CEN64Device *
CreateDevice(const char *pifROMPath) {
  struct AIFController *aif;
  struct BusController *bus;
  struct PIFController *pif;
  struct RDRAMController *rdram;
  struct ROMController *rom;
  struct VIFController *vif;

  struct CEN64Device *device;
  struct VR4300 *vr4300;
  struct RSP *rsp;

  if ((aif = CreateAIF()) == NULL)
    goto release_out;

  if ((pif = CreatePIF(pifROMPath)) == NULL)
    goto release_aif;

  if ((rom = CreateROM()) == NULL)
    goto release_pif;

  if ((rdram = CreateRDRAM()) == NULL)
    goto release_rdram;

  if ((rsp = CreateRSP()) == NULL)
    goto release_rom;

  if ((vif = CreateVIF()) == NULL)
    goto release_rsp;

  if ((vr4300 = CreateVR4300()) == NULL)
    goto release_vif;

  if ((bus = CreateBus(aif, pif, rdram, rom, rsp, vif, vr4300)) == NULL)
    goto release_vr4300;

  if ((device = (struct CEN64Device*) malloc(
    sizeof(struct CEN64Device))) == NULL) {
    debug("Failed to create all devices.");
    goto release_bus;
  }

  device->aif = aif;
  device->bus = bus;
  device->pif = pif;
  device->rdram = rdram;
  device->rom = rom;
  device->rsp = rsp;
  device->vif = vif;
  device->vr4300 = vr4300;
  device->cycles = 0;

  return device;

  /* Deallocate memory on failure. */
  release_bus:    DestroyBus(bus);
  release_vr4300: DestroyVR4300(vr4300);
  release_vif:    DestroyVIF(vif);
  release_rsp:    DestroyRSP(rsp);
  release_rom:    DestroyROM(rom);
  release_rdram:  DestroyRDRAM(rdram);
  release_pif:    DestroyPIF(pif);
  release_aif:    DestroyAIF(aif);
  release_out:

  return NULL;
}

/* ============================================================================
 *  CycleDevice: Advances the state of the device by one MasterClock cycle.
 * ========================================================================= */
void
CycleDevice(struct CEN64Device *device) {
  CycleAIF(device->aif);
  CycleVIF(device->vif);
  CycleRSP(device->rsp);

  CycleVR4300(device->vr4300);
  if (device->cycles & 0x01)
    CycleVR4300(device->vr4300);

  device->cycles++;
}

/* ============================================================================
 *  DestroyDevice: Releases any resources allocated for an instance.
 * ========================================================================= */
void
DestroyDevice(struct CEN64Device *device) {
  DestroyAIF(device->aif);
  DestroyBus(device->bus);
  DestroyPIF(device->pif);
  DestroyRDRAM(device->rdram);
  DestroyROM(device->rom);
  DestroyVIF(device->vif);
  DestroyRSP(device->rsp);
  DestroyVR4300(device->vr4300);

  free(device);
}

/* ============================================================================
 *  LoadCartridge; Simulates inserting a cartridge into the device.
 * ========================================================================= */
int
LoadCartridge(struct CEN64Device *device, const char *filename) {
  uint32_t seed;

  if (InsertCart(device->rom, filename))
    return 1;

  if ((seed = GetCICSeed(device->rom)) == 0)
    return 2;

  SetCICSeed(device->pif, seed);
  return 0;
}

