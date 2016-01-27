//
// ai/controller.c: Audio interface controller.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "ai/context.h"
#include "ai/controller.h"
#include "bus/address.h"
#include "bus/controller.h"
#include "ri/controller.h"
#include "vr4300/interface.h"

#define NTSC_DAC_FREQ 48681812 // 48.681812MHz

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *ai_register_mnemonics[NUM_AI_REGISTERS] = {
#define X(reg) #reg,
#include "ai/registers.md"
#undef X
};
#endif

static void ai_dma(struct ai_controller *ai);

// Advances the controller by one clock cycle.
void ai_cycle(struct ai_controller *ai) {
  if (likely(ai->counter-- != 0))
    return;

  // DMA engine is finishing up with one entry.
  if (ai->fifo_count > 0) {
    struct bus_controller *bus;

    memcpy(&bus, ai, sizeof(bus));
    signal_rcp_interrupt(bus->vr4300, MI_INTR_AI);

    ai->fifo_ri = (ai->fifo_ri == 1) ? 0 : 1;
    ai->regs[AI_STATUS_REG] &= ~0xC0000001;
    ai->counter = 0xFFFFFFFFU;
    ai->fifo_count--;

    if (ai->fifo_count > 0)
      ai_dma(ai);
  }
}

// Performs an (instantaneous) DMA.
void ai_dma(struct ai_controller *ai) {
  struct bus_controller *bus;

  // Shove things into the audio context, slide the window.
  memcpy(&bus, ai, sizeof(bus));

  if (ai->fifo[ai->fifo_ri].length > 0) {
    unsigned freq = (double) NTSC_DAC_FREQ / (ai->regs[AI_DACRATE_REG] + 1);
    unsigned samples = ai->fifo[ai->fifo_ri].length / 4;

    // Need to raise an interrupt when the DMA engine
    // is doing the last 8-byte data bus transfer...
    //
    // XXX: Should be > 2, I think, but don't want to
    // risk breaking things and would need to verify.
    ai->counter = (62500000.0 / freq) * (samples - 2);

    if (likely(ai->fifo[ai->fifo_ri].length > 0)) {

      // Shovel things into the audio context.
      uint8_t buf[0x40000];
      unsigned i;

      ALuint buffer;
      ALint val;

      alGetSourcei(ai->ctx.source, AL_BUFFERS_PROCESSED, &val);

      // XXX: Most games pick one frequency and stick with it.
      // Instead of paying garbage, try to dynamically switch
      // the frequency of the buffers that OpenAL is using.
      //
      // This will result in pops and other unpleasant things
      // when the frequency changes underneath us, but it still
      // seems to sound better than what we had before.
      if (ai->ctx.cur_frequency != freq) {
        if (val == sizeof(ai->ctx.buffers) / sizeof(ai->ctx.buffers[0])) {
          printf("OpenAL: Switching context buffer frequency to: %u\n", freq);
          ai_switch_frequency(&ai->ctx, freq);
        }

        else
          val = 0;
      }

      if (val) {
        for (i = 0; i < ai->fifo[ai->fifo_ri].length / 4; i++) {
          uint32_t word;

          // Byteswap audio buffer to native format.
          // TODO: Can we specify endian-ness somehow?
          memcpy(&word, bus->ri->ram + (ai->fifo[
          ai->fifo_ri].address + sizeof(word) * i), sizeof(word));
          word = byteswap_32(word);
          word = (word >> 16) | (word << 16);
          memcpy(buf + sizeof(word) * i, &word, sizeof(word));
        }

        if (ai->ctx.unqueued_buffers > 0) {
          buffer = ai->ctx.buffers[sizeof(ai->ctx.buffers) /
            sizeof(ai->ctx.buffers[0]) - ai->ctx.unqueued_buffers];

          ai->ctx.unqueued_buffers--;
        }

        else
          alSourceUnqueueBuffers(ai->ctx.source, 1, &buffer);

        alBufferData(buffer, AL_FORMAT_STEREO16, buf,
          ai->fifo[ai->fifo_ri].length, freq);
        alSourceQueueBuffers(ai->ctx.source, 1, &buffer);

        if (ai->ctx.unqueued_buffers == 1) {
          alSourcePlay(ai->ctx.source);
        }

        else {
          alGetSourcei(ai->ctx.source, AL_SOURCE_STATE, &val);

          if (val != AL_PLAYING)
            alSourcePlay(ai->ctx.source);
        }

        if (alGetError() != AL_NO_ERROR) {
          fprintf(stderr, "OpenAL: Reporting an error while playing sources!\n");
          fprintf(stderr, "Disabling it from this point forward; sorry!\n");
          ai->no_output = true;
        }
      }
    }
  }

  // If the length was zero, just interrupt now?
  else {
    signal_rcp_interrupt(bus->vr4300, MI_INTR_AI);

    ai->fifo_ri = (ai->fifo_ri == 1) ? 0 : 1;
    ai->regs[AI_STATUS_REG] &= ~0x80000001;
    ai->counter = 0xFFFFFFFFU;
    ai->fifo_count--;
  }

  if (ai->fifo_count > 0)
    ai->regs[AI_STATUS_REG] |= 0x40000000;
}

// Initializes the AI.
int ai_init(struct ai_controller *ai,
  struct bus_controller *bus, bool no_interface) {
  ai->bus = bus;

  ai->no_output = no_interface;

  if (!no_interface) {
    alGetError();

    if (ai_context_create(&ai->ctx))
      return 1;
  }

  // We'll recalculate once the first DMA is issued.
  ai->counter = 0xFFFFFFFFU;

  return 0;
}

// Reads a word from the AI MMIO register space.
int read_ai_regs(void *opaque, uint32_t address, uint32_t *word) {
  struct ai_controller *ai = (struct ai_controller *) opaque;
  unsigned offset = address - AI_REGS_BASE_ADDRESS;
  enum ai_register reg = (offset >> 2);

  *word = ai->regs[reg];
  debug_mmio_read(ai, ai_register_mnemonics[reg], *word);

  if (reg == AI_LEN_REG) {
    *word = 0;

    if (ai->regs[AI_STATUS_REG] & 0x80000001)
      *word = ai->regs[AI_LEN_REG];

    else if (ai->regs[AI_STATUS_REG] & 0x40000000) {
      // TODO
    }
  }

  return 0;
}

// Writes a word to the AI MMIO register space.
int write_ai_regs(void *opaque, uint32_t address, uint32_t word, uint32_t dqm) {
  struct ai_controller *ai = (struct ai_controller *) opaque;
  unsigned offset = address - AI_REGS_BASE_ADDRESS;
  enum ai_register reg = (offset >> 2);

  debug_mmio_write(ai, ai_register_mnemonics[reg], word, dqm);

  if (reg == AI_DRAM_ADDR_REG)
    ai->regs[AI_DRAM_ADDR_REG] = word & 0xFFFFF8;

  else if (reg == AI_LEN_REG) {
    ai->regs[AI_LEN_REG] = word & 0x3FFF8;

    if (ai->fifo_count == 2)
      return 0;

    // Fill the next FIFO entry in the DMA engine.
    ai->fifo[ai->fifo_wi].address = ai->regs[AI_DRAM_ADDR_REG];
    ai->fifo[ai->fifo_wi].length = ai->regs[AI_LEN_REG];
    ai->fifo_wi = (ai->fifo_wi == 1) ? 0 : 1;
    ai->fifo_count++;

    if (ai->fifo_count == 2)
      ai->regs[AI_STATUS_REG] |= 0x80000001U;

    // If we're not DMA-ing already, start DMA engine.
    if (!(ai->regs[AI_STATUS_REG] & 0x40000000U))
      ai_dma(ai);
  }

  else if (reg == AI_STATUS_REG) {
    struct bus_controller *bus;

    memcpy(&bus, ai, sizeof(bus));
    clear_rcp_interrupt(bus->vr4300, MI_INTR_AI);
  }

  else if (reg == AI_DACRATE_REG) {
    ai->regs[AI_DACRATE_REG] = word & 0x3FFF;

    ai->ctx.frequency = (double) NTSC_DAC_FREQ /
      (ai->regs[AI_DACRATE_REG] + 1);
  }

  else if (reg == AI_BITRATE_REG)
    ai->regs[AI_BITRATE_REG] = word & 0xF;

  else
    ai->regs[reg] = word;

  return 0;
}

