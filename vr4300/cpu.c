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

// Sets the opaque pointer used for external accesses.
static void vr4300_connect_bus(struct vr4300 *vr4300,
  struct bus_controller *bus) {
  vr4300->bus = bus;
}

// Initializes the VR4300 component.
int vr4300_init(struct vr4300 *vr4300, struct bus_controller *bus) {
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

