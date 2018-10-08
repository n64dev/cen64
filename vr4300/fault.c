//
// vr4300/fault.c: VR4300 fault management.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "bus/controller.h"
#include "vr4300/cp0.h"
#include "vr4300/cpu.h"
#include "vr4300/dcache.h"
#include "vr4300/fault.h"
#include "vr4300/icache.h"
#include "vr4300/pipeline.h"

const char *vr4300_fault_mnemonics[NUM_VR4300_FAULTS] = {
#define X(fault) #fault,
#include "vr4300/fault.md"
#undef X
};

static void vr4300_exception_prolog(struct vr4300 *vr4300,
  const struct vr4300_latch *l, uint32_t *cause, uint32_t *status,
  uint64_t *epc);

static void vr4300_tlb_exception_prolog(struct vr4300 *vr4300,
  const struct vr4300_latch *l, uint32_t *cause, uint32_t *status,
  uint64_t *epc, const struct segment *segment, uint64_t vaddr,
  unsigned *offs);

static void vr4300_exception_epilogue(struct vr4300 *vr4300,
  uint32_t cause, uint32_t status, uint64_t epc, uint64_t offs);

// Sets attributes common to all interlocks.
static void vr4300_common_interlocks(struct vr4300 *vr4300,
  unsigned cycles_to_stall, unsigned skip_stages) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  pipeline->cycles_to_stall = cycles_to_stall;
  vr4300->regs[PIPELINE_CYCLE_TYPE] = skip_stages;
}

// Raise a fault that originated in the DC stage.
static void vr4300_dc_fault(struct vr4300 *vr4300, enum vr4300_fault_id fault) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  pipeline->dcwb_latch.common.fault = fault;
  pipeline->exdc_latch.common.fault = fault;
  pipeline->rfex_latch.common.fault = fault;
  pipeline->icrf_latch.common.fault = fault;
}

// Raise a fault that originated in the EX stage.
static void vr4300_ex_fault(struct vr4300 *vr4300, enum vr4300_fault_id fault) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  pipeline->exdc_latch.common.fault = fault;
  pipeline->rfex_latch.common.fault = fault;
  pipeline->icrf_latch.common.fault = fault;
}

// Raise a fault that originated in the EX stage.
static void vr4300_rf_fault(struct vr4300 *vr4300, enum vr4300_fault_id fault) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  pipeline->rfex_latch.common.fault = fault;
  pipeline->icrf_latch.common.fault = fault;
}

// Prolog for most exceptions: load CP0 registers.
static void vr4300_exception_prolog(struct vr4300 *vr4300,
  const struct vr4300_latch *l, uint32_t *cause, uint32_t *status,
  uint64_t *epc) {

  bool in_bd_slot = l->cause_data >> 31;

  *status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  *cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];
  *epc = vr4300->regs[VR4300_CP0_REGISTER_EPC];

  if (!(*status & 0x2)) {
    if (in_bd_slot) {
      *cause |= 0x80000000U;
      *epc = l->pc - 4;
    }

    else {
      *cause &= ~0x80000000U;
      *epc = l->pc;
    }
  }
}

// Prolog for TLB exceptions: load/set CP0 registers.
void vr4300_tlb_exception_prolog(struct vr4300 *vr4300,
  const struct vr4300_latch *l, uint32_t *cause, uint32_t *status,
  uint64_t *epc, const struct segment *segment,
  uint64_t vaddr, unsigned *offs) {

  bool in_bd_slot = l->cause_data >> 31;
  uint64_t entryhi;
  uint64_t context;

  *status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];
  *cause = vr4300->regs[VR4300_CP0_REGISTER_CAUSE];
  *epc = vr4300->regs[VR4300_CP0_REGISTER_EPC];

  // Record branch delay slot?
  if (in_bd_slot) {
    if (!(*status & 0x2)) {
      *cause |= 0x80000000U;
      *epc = l->pc - 4;

      *offs = (*status & segment->xmode_mask) ? 0x080 : 0x000;
    }

    // MIPS says 0x80, but that doesn't make
    // any sense, so let's use 0x180 (GPE).
    else
      *offs = 0x180;
  }

  else {
    if (!(*status & 0x2)) {
      *cause &= ~0x80000000U;
      *epc = l->pc;

      *offs = (*status & segment->xmode_mask) ? 0x080 : 0x000;
    }

    // MIPS says 0x80, but that doesn't make
    // any sense, so let's use 0x180 (GPE).
    else
      *offs = 0x180;
  }

  // Write registers with exception data.
  if (*status & segment->xmode_mask) {
    uint64_t vpn2 = vaddr >> 13 & 0x7FFFFFF;
    uint8_t asid = vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI];
    uint8_t region = vaddr >> 62 & 0x3;

    entryhi = ((uint64_t) region << 62) | (vpn2 << 13) | asid;

    context = vr4300->regs[VR4300_CP0_REGISTER_XCONTEXT];
    context &= ~(0x1FFFFFFFULL << 4);
    context |= (uint64_t) region << 31;
    context |= vpn2 << 4;

    vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI] = entryhi;
    vr4300->regs[VR4300_CP0_REGISTER_XCONTEXT] = context;
  }

  else {
    uint32_t vpn2 = vaddr >> 13 & 0x7FFFF;
    uint8_t asid = vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI];

    entryhi = (int32_t) ((vpn2 << 13) | asid);

    context = vr4300->regs[VR4300_CP0_REGISTER_CONTEXT];
    context &= ~(0x7FFFFULL << 4);
    context |= vpn2 << 4;

    vr4300->regs[VR4300_CP0_REGISTER_ENTRYHI] = entryhi;
    vr4300->regs[VR4300_CP0_REGISTER_CONTEXT] = context;
  }

  vr4300->regs[VR4300_CP0_REGISTER_BADVADDR] = vaddr;
}

// Epilogue for all CPU exceptions.
void vr4300_exception_epilogue(struct vr4300 *vr4300,
  uint32_t cause, uint32_t status, uint64_t epc, uint64_t offs) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  // Reset the segment ptrs as we might change access level.
  pipeline->icrf_latch.segment = get_default_segment();
  pipeline->exdc_latch.segment = get_default_segment();

  // Reset the exception history count, signal the presence
  // of a fault, and stall for the mandatory cycle count.
  //
  // TODO: Is the cycle count just the killing of IC/RF,
  // or do we actually delay an additional two cycles?
  vr4300->regs[PIPELINE_CYCLE_TYPE] = 0;
  pipeline->exception_history = 0;
  pipeline->fault_present = true;
  pipeline->cycles_to_stall = 2;

  // Set CP0 registers in accordance with the exception.
  if (vr4300->regs[VR4300_CP0_REGISTER_STATUS] & 0x2) {
    vr4300->regs[VR4300_CP0_REGISTER_STATUS] = status | 0x4;
    vr4300->regs[VR4300_CP0_REGISTER_ERROREPC] = epc;
  }

  else {
    vr4300->regs[VR4300_CP0_REGISTER_STATUS] = status | 0x2;
    vr4300->regs[VR4300_CP0_REGISTER_EPC] = epc;
  }

  vr4300->regs[VR4300_CP0_REGISTER_CAUSE] = cause;

  vr4300->pipeline.icrf_latch.pc = ((status & 0x400000)
    ? (0xFFFFFFFFBFC00200ULL + offs)
    : (0xFFFFFFFF80000000ULL + offs)
  );
}

// CPU: Coprocessor unusable exception.
void VR4300_CPU(struct vr4300 *vr4300) {
  struct vr4300_latch *common = &vr4300->pipeline.exdc_latch.common;
  uint32_t cause, status;
  uint64_t epc;

  vr4300_ex_fault(vr4300, VR4300_FAULT_CPU);
  vr4300_exception_prolog(vr4300, common, &cause, &status, &epc);
  vr4300_exception_epilogue(vr4300, (cause & ~0xFF) | (1 << 28) | 0x2C,
    status, epc, 0x180);
}

// DADE: Data address error exception.
void VR4300_DADE(struct vr4300 *vr4300) {
  abort(); // Hammertime!
}

// DCB: Data cache busy interlock.
void VR4300_DCB(struct vr4300 *vr4300) {
  vr4300->pipeline.dcwb_latch.last_op_was_cache_store = false;
  vr4300_common_interlocks(vr4300, 0, 1);
}

// DCM: Data cache busy interlock.
void VR4300_DCM(struct vr4300 *vr4300) {
  struct vr4300_dcwb_latch *dcwb_latch = &vr4300->pipeline.dcwb_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  struct vr4300_bus_request *request = &exdc_latch->request;

  uint64_t vaddr = request->vaddr;
  uint32_t paddr = request->paddr;
  struct vr4300_dcache_line *line;
  uint32_t data[4];
  unsigned i;

  if (!exdc_latch->cached) {
    unsigned mask = request->access_type ==
      VR4300_ACCESS_DWORD ? 0x7 : 0x3;

    // Service a read.
    if (exdc_latch->request.type == VR4300_BUS_REQUEST_READ) {
      unsigned rshiftamt = (8 - request->size) << 3;
      unsigned lshiftamt = (paddr & mask) << 3;
      uint32_t hiword, loword;
      int64_t sdata;

      paddr &= ~mask;
      bus_read_word(vr4300->bus, paddr, &hiword);

      if (request->access_type != VR4300_ACCESS_DWORD)
        sdata = (uint64_t) hiword << (lshiftamt + 32);

      else {
        bus_read_word(vr4300->bus, paddr + 4, &loword);
        sdata = ((uint64_t) hiword << 32) | loword;
        sdata = sdata << lshiftamt;
      }

      // TODO: rdqm?!
      dcwb_latch->result |= (sdata >> rshiftamt &
        request->data) << request->postshift;
    }

    // Service a write.
    else {
      uint64_t data = request->data;
      uint64_t dqm = request->wdqm;

      paddr &= ~mask;

      if (request->access_type == VR4300_ACCESS_DWORD) {
        bus_write_word(vr4300->bus, paddr, data >> 32, dqm >> 32);
        paddr += 4;
      }
 
      bus_write_word(vr4300->bus, paddr, data, dqm);
    }

    vr4300_common_interlocks(vr4300, MEMORY_WORD_DELAY, 2);
    return;
  }

  // Cached accesses require us to potentially flush the old line.
  // In addition to that, we also need to pull in the next one.
  if ((line = vr4300_dcache_should_flush_line(
    &vr4300->dcache, vaddr)) != NULL) {
    uint32_t bus_address;

    bus_address = vr4300_dcache_get_tag(line, vaddr);
    memcpy(data, line->data, sizeof(data));

    for (i = 0; i < 4; i++)
      bus_write_word(vr4300->bus, bus_address + i * 4,
        data[i ^ (WORD_ADDR_XOR >> 2)], ~0);
  }

  // Raise interlock condition, get virtual address.
  vr4300_common_interlocks(vr4300, DCACHE_ACCESS_DELAY, 1);
  paddr &= ~0xF;

  // Fill the cache line.
  for (i = 0; i < 4; i++)
    bus_read_word(vr4300->bus, paddr + i * 4,
      data + (i ^ (WORD_ADDR_XOR >> 2)));

  vr4300_dcache_fill(&vr4300->dcache, vaddr, paddr, data);
}

// DTLB: Data TLB exception.
void VR4300_DTLB(struct vr4300 *vr4300, unsigned miss, unsigned inv, unsigned mod) {
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  struct vr4300_latch *common = &exdc_latch->common;
  uint32_t cause, status;
  uint64_t epc;

  unsigned offs, type;

  // TLB miss/invalid exceptions are either TLBL or TLBS.
  if (miss | inv)
    type = (exdc_latch->request.type == VR4300_BUS_REQUEST_WRITE) ? 0x3: 0x2;

  // OTOH, TLB modification exceptions are TLBM.
  else
    type = 0x1;

  vr4300_dc_fault(vr4300, VR4300_FAULT_DTLB);
  vr4300_tlb_exception_prolog(vr4300, common, &cause, &status,
    &epc, exdc_latch->segment, exdc_latch->request.vaddr, &offs);

  // We calculated the vector offset for TLB miss exceptions.
  // TLB invalid and modification exceptions always use the GPE.
  if (!miss && !mod)
    offs = 0x180;

  vr4300_exception_epilogue(vr4300, (cause & ~0xFF) | (type << 2),
    status, epc, offs);
}

// IADE: Instruction address error exception.
void VR4300_IADE(struct vr4300 *vr4300) {
  abort(); // Hammertime!
}

// ICB: Instruction cache busy interlock.
void VR4300_ICB(struct vr4300 *vr4300) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  uint64_t vaddr = icrf_latch->common.pc;
  uint32_t paddr = rfex_latch->paddr;
  unsigned delay;

  if (!rfex_latch->cached) {
    bus_read_word(vr4300->bus, paddr, &rfex_latch->iw);
    delay = MEMORY_WORD_DELAY;
  }

  else {
    uint32_t line[8];
    unsigned i;

    paddr &= ~0x1C;

    // Fill the cache line.
    for (i = 0; i < 8; i ++)
      bus_read_word(vr4300->bus, paddr + i * 4, line + i);

    memcpy(&rfex_latch->iw, line + (vaddr >> 2 & 0x7), sizeof(rfex_latch->iw));
    vr4300_icache_fill(&vr4300->icache, icrf_latch->common.pc, paddr, line);
    delay = ICACHE_ACCESS_DELAY;
  }

  vr4300_common_interlocks(vr4300, delay, 4);
}

// INTR: Interrupt exception.
void VR4300_INTR(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_latch *common = &pipeline->dcwb_latch.common;
  uint32_t cause, status;
  uint64_t epc;

  vr4300_exception_prolog(vr4300, common, &cause, &status, &epc);
  vr4300_exception_epilogue(vr4300, cause & ~0xFF, status, epc, 0x180);

  vr4300_dc_fault(vr4300, VR4300_FAULT_INTR);
}

// INV: Invalid operation exception.
void VR4300_INV(struct vr4300 *vr4300) {
  abort(); // Hammertime!
}

// ITLB: Instruction TLB exception.
void VR4300_ITLB(struct vr4300 *vr4300, unsigned miss) {
  struct vr4300_icrf_latch *icrf_latch = &vr4300->pipeline.icrf_latch;
  struct vr4300_latch *common = &icrf_latch->common;
  uint32_t cause, status;
  uint64_t epc;

  unsigned offs;

  vr4300_rf_fault(vr4300, VR4300_FAULT_ITLB);
  vr4300_tlb_exception_prolog(vr4300, common, &cause, &status,
    &epc, icrf_latch->segment, icrf_latch->common.pc, &offs);

  // Invalid TLB exceptions always use the GPE.
  if (!miss)
    offs = 0x180;

  vr4300_exception_epilogue(vr4300, (cause & ~0xFF) | (0x2 << 2),
    status, epc, offs);
}

// LDI: Load delay interlock.
void VR4300_LDI(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_exdc_latch *exdc_latch = &pipeline->exdc_latch;

  // We'll do EX again, but clear the 'busy' flag.
  exdc_latch->request.type = VR4300_BUS_REQUEST_NONE;
  vr4300_common_interlocks(vr4300, 0, 2);
}

// RST: External reset exception.
void VR4300_RST(struct vr4300 *vr4300) {

  // Cold reset exception.
  if (vr4300->signals & VR4300_SIGNAL_COLDRESET) {
    vr4300->signals &= ~VR4300_SIGNAL_COLDRESET;

    vr4300->regs[VR4300_CP0_REGISTER_STATUS] &= ~0x08300000ULL;
    vr4300->regs[VR4300_CP0_REGISTER_CONFIG] &= ~0xFFFF7FF0ULL;

    vr4300->regs[VR4300_CP0_REGISTER_STATUS] |= 0x00400004ULL;
    vr4300->regs[VR4300_CP0_REGISTER_CONFIG] |= 0x7006E460ULL;

    vr4300->regs[VR4300_CP0_REGISTER_RANDOM] = 31;
  }

  // Soft reset exception.
  else
    abort();

  vr4300_dc_fault(vr4300, VR4300_FAULT_RST);

  vr4300_exception_epilogue(vr4300,
    vr4300->regs[VR4300_CP0_REGISTER_CAUSE],
    vr4300->regs[VR4300_CP0_REGISTER_STATUS],
    vr4300->regs[VR4300_CP0_REGISTER_EPC],
    -0x200ULL);
}

// SYSC: System call exception.
void VR4300_SYSC(struct vr4300 *vr4300) {
  struct vr4300_latch *common = &vr4300->pipeline.exdc_latch.common;
  uint32_t cause, status;
  uint64_t epc;

  vr4300_ex_fault(vr4300, VR4300_FAULT_SYSC);
  vr4300_exception_prolog(vr4300, common, &cause, &status, &epc);
  vr4300_exception_epilogue(vr4300, (cause & ~0xFF) | 0x20,
    status, epc, 0x180);
}

// WAT: Watch exception.
void VR4300_WAT(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;
  struct vr4300_latch *common = &pipeline->dcwb_latch.common;
  uint32_t cause, status;
  uint64_t epc;

  vr4300_exception_prolog(vr4300, common, &cause, &status, &epc);
  vr4300_exception_epilogue(vr4300, (cause & ~0xFF) | (23 << 2),
    status, epc, 0x180);

  vr4300_dc_fault(vr4300, VR4300_FAULT_WAT);
}

