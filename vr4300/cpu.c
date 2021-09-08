//
// vr4300/cpu.c: VR4300 processor container.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "vr4300/interface.h"
#include "vr4300/cp0.h"
#include "vr4300/cp1.h"
#include "vr4300/cpu.h"
#include "vr4300/icache.h"
#include "vr4300/pipeline.h"

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *mi_register_mnemonics[NUM_MI_REGISTERS] = {
#define X(reg) #reg,
#include "vr4300/registers.md"
#undef X
};
#endif

void vr4300_cycle(struct vr4300 *vr4300) {
  struct vr4300_pipeline *pipeline = &vr4300->pipeline;

  // Increment counters.
  vr4300->regs[VR4300_CP0_REGISTER_COUNT]++;

  // We're stalling for something...
  if (pipeline->cycles_to_stall > 0)
    pipeline->cycles_to_stall--;

  else
    vr4300_cycle_(vr4300);

  if ((vr4300->regs[VR4300_CP0_REGISTER_COUNT] & 1) == 1 &&
    (uint32_t) (vr4300->regs[VR4300_CP0_REGISTER_COUNT] >> 1) ==
    (uint32_t) vr4300->regs[VR4300_CP0_REGISTER_COMPARE]) {
    vr4300->regs[VR4300_CP0_REGISTER_CAUSE] |= 0x8000;
  }
}

// Sets the opaque pointer used for external accesses.
static void vr4300_connect_bus(struct vr4300 *vr4300,
  struct bus_controller *bus) {
  vr4300->bus = bus;
}

// Initializes the VR4300 component.
int vr4300_init(struct vr4300 *vr4300, struct bus_controller *bus, bool profiling) {
  vr4300_connect_bus(vr4300, bus);

  vr4300_cp0_init(vr4300);
  vr4300_cp1_init(vr4300);

  vr4300_dcache_init(&vr4300->dcache);
  vr4300_icache_init(&vr4300->icache);

  vr4300_pipeline_init(&vr4300->pipeline);
  vr4300->signals = VR4300_SIGNAL_COLDRESET;

  // MESS uses this version, so we will too?
  vr4300->mi_regs[MI_VERSION_REG] = 0x01010101;
  vr4300->mi_regs[MI_INIT_MODE_REG] = 0x80;

  if (profiling)
    vr4300->profile_samples = calloc(2 * 8 * 1024 * 1024, sizeof(uint64_t));
  else
    vr4300->profile_samples = NULL;

  debug_init(&vr4300->debug, DEBUG_SOURCE_VR4300);

  return 0;
}

// Prints out simulation information to stdout.
void vr4300_print_summary(struct vr4300_stats *stats) {
  unsigned i, j;
  float secs;
  float cpi;

  // Print banner.
  printf("###############################\n"
         " NEC VR4300 Simulation Summary\n"
         "###############################\n"
         "\n"
  );

  // Print configuration, summary, whatever.
  secs = stats->total_cycles / 93750000.0f;

  printf("   %16s: %.1f sec.\n"
         "\n\n",

    "Actual runtime", secs
  );

  // Print performance statistics.
  cpi = (float) stats->executed_instructions / stats->total_cycles;

  printf(" * Performance statistics:\n\n"
         "   %16s: %lu\n"
         "   %16s: %lu\n"
         "   %16s: %1.2f\n"
         "\n\n",

    "Elapsed pcycles", stats->total_cycles,
    "Insns executed", stats->executed_instructions,
    "Average CPI", cpi
  );

  // Print executed opcode counts.
  printf(" * Executed instruction counts:\n\n");

  for (i = 1; i < NUM_VR4300_OPCODES; i += 2) {
    for (j = 0; i + j < NUM_VR4300_OPCODES && j < 2; j++) {
      printf("   %16s: %16lu", vr4300_opcode_mnemonics[i + j],
        stats->opcode_counts[i + j]);

      if (j == 0)
        printf("\t");
    }

    printf("\n");
  }
}

uint64_t vr4300_get_register(struct vr4300 *vr4300, size_t i) {
  return vr4300->regs[i];
}

uint64_t vr4300_get_pc(struct vr4300 *vr4300) {
  return vr4300->pipeline.dcwb_latch.common.pc;
}

cen64_cold void vr4300_signal_break(struct vr4300 *vr4300) {
  debug_signal(&vr4300->debug, DEBUG_SIGNALS_BREAK);
}

cen64_cold void vr4300_set_breakpoint(struct vr4300 *vr4300, uint64_t at) {
  debug_set_breakpoint(&vr4300->debug, at);
}

cen64_cold void vr4300_remove_breakpoint(struct vr4300 *vr4300, uint64_t at) {
  debug_remove_breakpoint(&vr4300->debug, at);
}

struct vr4300* vr4300_alloc() {
    struct vr4300* ptr = (struct vr4300*)malloc(sizeof(struct vr4300));
    memset(ptr, 0, sizeof(struct vr4300));
    return ptr;
}

cen64_cold void vr4300_free(struct vr4300* ptr) {
    debug_cleanup(&ptr->debug);
    free(ptr);
}

cen64_cold struct vr4300_stats* vr4300_stats_alloc() {
    struct vr4300_stats* ptr = (struct vr4300_stats*)malloc(sizeof(struct vr4300_stats));
    memset(ptr, 0, sizeof(struct vr4300_stats));
    return ptr;
}

cen64_cold void vr4300_stats_free(struct vr4300_stats* ptr) {
    free(ptr);
}

cen64_cold void vr4300_connect_debugger(struct vr4300 *vr4300, void* break_handler_data, debug_break_handler break_handler) {
  vr4300->debug.break_handler = break_handler;
  vr4300->debug.break_handler_data = break_handler_data;
}