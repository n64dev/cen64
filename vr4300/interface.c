//
// vr4300/interface.c: VR4300 interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "vr4300/cpu.h"
#include "vr4300/interface.h"

#define MI_EBUS_TEST_MODE 0x0080
#define MI_INIT_MODE      0x0100
#define MI_RDRAM_REG_MODE 0x0200

static void lower_rcp_interrupt(struct vr4300 *vr4300);
static void raise_rcp_interrupt(struct vr4300 *vr4300);

// Checks for interrupts, possibly sets the cause bit.
static void check_for_interrupts(struct vr4300 *vr4300) {
  if (vr4300->mi_regs[MI_INTR_REG] & vr4300->mi_regs[MI_INTR_MASK_REG])
    raise_rcp_interrupt(vr4300);
  else
    lower_rcp_interrupt(vr4300);
}

// Callback: An RCP component is clearing an interrupt.
void clear_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask) {
  vr4300->mi_regs[MI_INTR_REG] &= ~mask;
  check_for_interrupts(vr4300); // TODO/FIXME: ???
}

// Deasserts the interrupt signal from the RCP.
static void lower_rcp_interrupt(struct vr4300 *vr4300) {
  vr4300->regs[VR4300_CP0_REGISTER_CAUSE] &= ~0x400;
}

// Asserts the interrupt signal from the RCP.
static void raise_rcp_interrupt(struct vr4300 *vr4300) {
  vr4300->regs[VR4300_CP0_REGISTER_CAUSE] |= 0x400;
}

// Deasserts the interrupt signal from the 64DD.
void clear_dd_interrupt(struct vr4300 *vr4300) {
  vr4300->regs[VR4300_CP0_REGISTER_CAUSE] &= ~0x800;
}

// Asserts the interrupt signal from the 64DD.
void signal_dd_interrupt(struct vr4300 *vr4300) {
  vr4300->regs[VR4300_CP0_REGISTER_CAUSE] |= 0x800;
}

// Callback: An RCP component is signaling an interrupt.
void signal_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask) {
  vr4300->mi_regs[MI_INTR_REG] |= mask;
  check_for_interrupts(vr4300); // TODO/FIXME: ???
}

// Reads a word from the MI MMIO register space.
int read_mi_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct vr4300 *vr4300 = (struct vr4300 *) opaque;
  uint32_t offset = address - MI_REGS_BASE_ADDRESS;
  enum mi_register reg = (offset >> 2);

  *word = vr4300->mi_regs[reg];
  debug_mmio_read(mi, mi_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the MI MMIO register space.
int write_mi_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct vr4300 *vr4300 = (struct vr4300 *) opaque;
  uint32_t offset = address - MI_REGS_BASE_ADDRESS;
  enum mi_register reg = (offset >> 2);
  uint32_t result;

  debug_mmio_write(mi, mi_register_mnemonics[reg], word, dqm);

  // Change mode settings?
  if (reg == MI_INIT_MODE_REG) {
    result = word & 0x3FF;

    if (word & 0x0080)
      result &= ~MI_INIT_MODE;
    else if (word & 0x0100)
      result |= MI_INIT_MODE;

    if (word & 0x0200)
      result &= ~MI_EBUS_TEST_MODE;
    else if (word & 0x0400)
      result |= MI_EBUS_TEST_MODE;

    if (word & 0x0800) {
      vr4300->mi_regs[MI_INTR_REG] &= ~MI_INTR_DP;
      check_for_interrupts(vr4300); // TODO/FIXME: ???
    }

    if (word & 0x1000)
      result &= ~MI_RDRAM_REG_MODE;
    else if (word & 0x2000)
      result |= MI_RDRAM_REG_MODE;

    vr4300->mi_regs[MI_INIT_MODE_REG] = result;
  }

  // Change interrupt mask?
  else if (reg == MI_INTR_MASK_REG) {
    if (word & 0x0001)
      vr4300->mi_regs[MI_INTR_MASK_REG] &= ~MI_INTR_SP;
    else if (word & 0x0002)
      vr4300->mi_regs[MI_INTR_MASK_REG] |= MI_INTR_SP;

    if (word & 0x0004)
      vr4300->mi_regs[MI_INTR_MASK_REG] &= ~MI_INTR_SI;
    else if (word & 0x0008)
      vr4300->mi_regs[MI_INTR_MASK_REG] |= MI_INTR_SI;

    if (word & 0x0010)
      vr4300->mi_regs[MI_INTR_MASK_REG] &= ~MI_INTR_AI;
    else if (word & 0x0020)
      vr4300->mi_regs[MI_INTR_MASK_REG] |= MI_INTR_AI;

    if (word & 0x0040)
      vr4300->mi_regs[MI_INTR_MASK_REG] &= ~MI_INTR_VI;
    else if (word & 0x0080)
      vr4300->mi_regs[MI_INTR_MASK_REG] |= MI_INTR_VI;

    if (word & 0x0100)
      vr4300->mi_regs[MI_INTR_MASK_REG] &= ~MI_INTR_PI;
    else if (word & 0x0200)
      vr4300->mi_regs[MI_INTR_MASK_REG] |= MI_INTR_PI;

    if (word & 0x0400)
      vr4300->mi_regs[MI_INTR_MASK_REG] &= ~MI_INTR_DP;
    else if (word & 0x0800)
      vr4300->mi_regs[MI_INTR_MASK_REG] |= MI_INTR_DP;

    check_for_interrupts(vr4300); // TODO/FIXME: ???
  }

  else {
    vr4300->mi_regs[reg] &= ~dqm;
    vr4300->mi_regs[reg] |= word;
  }

  return 0;
}

