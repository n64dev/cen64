//
// vr4300/interface.c: VR4300 interface.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "vr4300/cpu.h"
#include "vr4300/interface.h"
#ifdef _WIN32
#include <windows.h>
#endif

#define MI_EBUS_TEST_MODE 0x0080
#define MI_INIT_MODE      0x0100
#define MI_RDRAM_REG_MODE 0x0200

static void lower_rcp_interrupt(struct vr4300 *vr4300);
static void raise_rcp_interrupt(struct vr4300 *vr4300);

// Checks for interrupts, possibly sets the cause bit.
static void check_for_interrupts(struct vr4300 *vr4300, uint32_t flags) {
  if (flags & vr4300->mi_regs[MI_INTR_MASK_REG])
    raise_rcp_interrupt(vr4300);
  else
    lower_rcp_interrupt(vr4300);
}

// Callback: An RCP component is clearing an interrupt.
void clear_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask) {
#ifdef _MSC_VER
  uint32_t flags = InterlockedAnd(&vr4300->mi_regs[MI_INTR_REG], ~mask) & ~mask;
#else
  uint32_t flags = __sync_and_and_fetch(&vr4300->mi_regs[MI_INTR_REG], ~mask);
#endif
  check_for_interrupts(vr4300, flags); // TODO/FIXME: ???
}

// Deasserts the interrupt signal from the RCP.
static void lower_rcp_interrupt(struct vr4300 *vr4300) {
#ifdef _MSC_VER
  InterlockedAnd(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], ~0x400);
#else
  __sync_and_and_fetch(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], ~0x400);
#endif
}

// Asserts the interrupt signal from the RCP.
static void raise_rcp_interrupt(struct vr4300 *vr4300) {
#ifdef _MSC_VER
  InterlockedOr(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], 0x400);
#else
  __sync_or_and_fetch(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], 0x400);
#endif
}

// Deasserts the interrupt signal from the 64DD.
void clear_dd_interrupt(struct vr4300 *vr4300) {
#ifdef _MSC_VER
  InterlockedAnd(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], ~0x800);
#else
  __sync_and_and_fetch(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], ~0x800);
#endif
}

// Asserts the interrupt signal from the 64DD.
void signal_dd_interrupt(struct vr4300 *vr4300) {
#ifdef _MSC_VER
  InterlockedOr(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], 0x800);
#else
  __sync_or_and_fetch(&vr4300->regs[VR4300_CP0_REGISTER_CAUSE], 0x800);
#endif
}

// Callback: An RCP component is signaling an interrupt.
void signal_rcp_interrupt(struct vr4300 *vr4300, enum rcp_interrupt_mask mask) {
#ifdef _MSC_VER
  uint32_t flags = InterlockedOr(&vr4300->mi_regs[MI_INTR_REG], mask) | mask;
#else
  uint32_t flags = __sync_or_and_fetch(&vr4300->mi_regs[MI_INTR_REG], mask);
#endif
  check_for_interrupts(vr4300, flags); // TODO/FIXME: ???
}

// Reads a word from the MI MMIO register space.
int read_mi_regs(struct vr4300 *vr4300, uint32_t address, uint32_t *word) {
  uint32_t offset = address - MI_REGS_BASE_ADDRESS;
  enum mi_register reg = (offset >> 2);

  *word = vr4300->mi_regs[reg];
  debug_mmio_read(mi, mi_register_mnemonics[reg], *word);
  return 0;
}

// Writes a word to the MI MMIO register space.
int write_mi_regs(struct vr4300 *vr4300, uint32_t address, uint32_t word, uint32_t dqm) {
  uint32_t offset = address - MI_REGS_BASE_ADDRESS;
  enum mi_register reg = (offset >> 2);
  uint32_t result;
  uint32_t flags;

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
#ifdef _MSC_VER
      flags = InterlockedAnd(&vr4300->mi_regs[MI_INTR_REG], ~MI_INTR_DP) & ~MI_INTR_DP;
#else
      flags = __sync_and_and_fetch(&vr4300->mi_regs[MI_INTR_REG], ~MI_INTR_DP);
#endif
      check_for_interrupts(vr4300, flags); // TODO/FIXME: ???
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

    check_for_interrupts(vr4300, vr4300->mi_regs[MI_INTR_REG]); // TODO/FIXME: ???
  }

  else {
    vr4300->mi_regs[reg] &= ~dqm;
    vr4300->mi_regs[reg] |= word;
  }

  return 0;
}

int has_profile_samples(struct vr4300 const *vr4300)
{
    return vr4300->profile_samples != NULL;
}

uint64_t get_profile_sample(struct vr4300 const *vr4300, size_t i)
{
    return vr4300->profile_samples[i];
}

bool vr4300_read_word_vaddr(struct vr4300 *vr4300, uint64_t vaddr, uint32_t* result) {
  if (vaddr & 0x3) {
    // must be aligned
    return false;
  }

  const struct segment* segment = get_segment(vaddr, vr4300->regs[VR4300_CP0_REGISTER_STATUS]);

  if (!segment) {
    return false;
  }

  uint32_t paddr;
  bool cached;

  if (segment->mapped) {
    unsigned asid = vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI] & 0xFF;
    unsigned select, tlb_miss, index;
    uint32_t page_mask;

    tlb_miss = tlb_probe(&vr4300->cp0.tlb, vaddr, asid, &index);
    page_mask = vr4300->cp0.page_mask[index];
    select = ((page_mask + 1) & vaddr) != 0;

    if (unlikely(tlb_miss || !(vr4300->cp0.state[index][select] & 2))) {
      return false;
    }

    cached = ((vr4300->cp0.state[index][select] & 0x38) != 0x10);
    paddr = (vr4300->cp0.pfn[index][select]) | (vaddr & page_mask);
  } else {
    paddr = vaddr - segment->offset;
    cached = segment->cached;
  }

  if (cached) {
    struct vr4300_dcache_line* line = vr4300_dcache_probe(&vr4300->dcache, vaddr, paddr);

    if (line) {
      memcpy(result, line->data + ((paddr & 0xf) ^ WORD_ADDR_XOR), sizeof(uint32_t));
    } else {
      bus_read_word(vr4300->bus, paddr, result);
    }
  } else {
    bus_read_word(vr4300->bus, paddr, result);
  }

  return true;
}