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
#include "rsp/rsp.h"
#include "vr4300/interface.h"

#define NTSC_DAC_FREQ 48681812 // 48.681812MHz

#ifdef DEBUG_MMIO_REGISTER_ACCESS
const char *ai_register_mnemonics[NUM_AI_REGISTERS] = {
#define X(reg) #reg,
#include "ai/registers.md"
#undef X
};
#endif

static cen64_align(uint8_t buf[0x40000], 16);

static void ai_dma(struct ai_controller *ai);
static const uint8_t *byteswap_audio_buffer(const uint8_t *input,
    uint8_t *output, uint32_t length);

// Advances the controller by one clock cycle.
void ai_cycle_(struct ai_controller *ai) {

  // DMA engine is finishing up with one entry.
  if (ai->fifo_count > 0) {
    struct bus_controller *bus;

    memcpy(&bus, ai, sizeof(bus));
    signal_rcp_interrupt(bus->vr4300, MI_INTR_AI);

    ai->fifo_ri ^= 0x1;
    ai->regs[AI_STATUS_REG] &= ~0xC0000001;
    ai->fifo_count--;

    if (ai->fifo_count > 0) {
      ai->regs[AI_STATUS_REG] = 0x40000000;
      ai_dma(ai);
    }
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

    // Shovel things into the audio context.
    if (ai->no_output)
      ai->counter = (62500000.0 / freq) * samples;

    else {
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
        if (val == 0) {
          printf("OpenAL: Switching context buffer frequency to: %u\n", freq);
          ai_switch_frequency(&ai->ctx, freq);
        }
      }

      // Grab any buffers that have been processed (for reuse).
      if (val) {
        alSourceUnqueueBuffers(ai->ctx.source, val,
            ai->ctx.buffers + ai->ctx.unqueued_buffers);

        ai->ctx.unqueued_buffers += val;
      }

      // Need to raise an interrupt when the DMA engine
      // is doing the last 8-byte data bus transfer...

      // This dodgy, since the RCP's audio is more or less a realtime
      // (and CEN64 has *not* a realtime system, nor does it have a
      // resampler that can fudge the audio around non-perfectly-timed
      // INTs). So, for now, throw interrupts a little early to prevent
      // popping.
      switch (ai->ctx.unqueued_buffers) {

        // We are ahead of where we want to be. Keep spinning
        // until OpenAL has some free buffers for us.
        case 0:
        case 1:
          ai->counter = (62500000.0 / freq) * samples;
          break;

        // One unprocessed buffer, one going. Try to throw the
        // INT almost immediately when the current buffer finshes.
        case 2:
          ai->counter = (62500000.0 / freq) * (samples - 10);
          break;

        // No buffers, we should throw an INT almost immediately?
        case 3:
          ai->counter = 1;
          break;
      }

      if (ai->ctx.unqueued_buffers > 0) {
        uint32_t length = ai->fifo[ai->fifo_ri].length;
        uint8_t *input = bus->ri->ram + ai->fifo[ai->fifo_ri].address;
        const uint8_t *buf_ptr = byteswap_audio_buffer(input, buf, length);

        ai->ctx.unqueued_buffers--;
        buffer = ai->ctx.buffers[ai->ctx.unqueued_buffers];

        alBufferData(buffer, AL_FORMAT_STEREO16, buf_ptr, length, freq);
        alSourceQueueBuffers(ai->ctx.source, 1, &buffer);
      }

      alGetSourcei(ai->ctx.source, AL_SOURCE_STATE, &val);

      if (val != AL_PLAYING)
        alSourcePlay(ai->ctx.source);
    }
  }

  // If the length was zero, just interrupt now?
  else {
    signal_rcp_interrupt(bus->vr4300, MI_INTR_AI);

    ai->fifo_ri ^= 0x1;
    ai->regs[AI_STATUS_REG] &= ~0xC0000001;
    ai->fifo_count--;

    if (ai->fifo_count > 0) {
      ai->regs[AI_STATUS_REG] |= 0x40000000;
      ai->counter = 1;
    }
  }
}

// Initializes the AI.
int ai_init(struct ai_controller *ai,
  struct bus_controller *bus, bool no_interface) {
  ai->bus = bus;

  ai->no_output = no_interface;

  if (!no_interface) {
    alGetError();

    if (ai_context_create(&ai->ctx)) {
      ai->no_output = 1;
      return 1;
    }
  }

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
    ai->fifo_wi ^= 0x1;
    ai->fifo_count++;

    if (ai->fifo_count == 2)
      ai->regs[AI_STATUS_REG] |= 0x80000001U;

    // If we're not DMA-ing already, start DMA engine.
    if (!(ai->regs[AI_STATUS_REG] & 0x40000000U)) {
      ai->regs[AI_STATUS_REG] = 0x40000000;
      ai_dma(ai);
    }
  }

  else if (reg == AI_STATUS_REG) {
    struct bus_controller *bus;

    memcpy(&bus, ai, sizeof(bus));
    clear_rcp_interrupt(bus->vr4300, MI_INTR_AI);
  }

  else if (reg == AI_DACRATE_REG) {
    ai->regs[AI_DACRATE_REG] = word & 0x3FFF;

    ai->ctx.frequency = (double) NTSC_DAC_FREQ / (ai->regs[AI_DACRATE_REG] + 1);
  }

  else if (reg == AI_BITRATE_REG)
    ai->regs[AI_BITRATE_REG] = word & 0xF;

  else
    ai->regs[reg] = word;

  return 0;
}

const uint8_t *byteswap_audio_buffer(const uint8_t *input,
    uint8_t *output, uint32_t length) {
  uint32_t i = 0;

#ifdef __SSE2__
#ifdef __SSSE3__
  static const uint32_t byteswap_key[] = {
      0x02030001, 0x06070405, 0x0A0B0809, 0x0E0F0C0D};
  const __m128i key = _mm_load_si128((__m128i*) byteswap_key);
#endif
  const uint8_t *ret_buf;
  uintptr_t input_ptr;

  memcpy(&input_ptr, &input, sizeof(input_ptr));

  // Align input buffer to 16 bytes (since transfers are 8 bytes).
  if (input_ptr & 0x8) {
    __m128i samples = _mm_loadl_epi64((__m128i*) input);
#ifdef __SSSE3__
    samples = _mm_shuffle_epi8(samples, key);
#else
    __m128i swapleft = _mm_slli_epi16(samples, 8);
    __m128i swapright = _mm_srli_epi16(samples, 8);
    samples = _mm_or_si128(swapleft, swapright);
#endif
    _mm_storel_epi64((__m128i*) (output + sizeof(uint64_t)), samples);

    output += sizeof(uint64_t);
    i += sizeof(uint64_t);
  }

  ret_buf = output;

  // Byteswap the majority of the buffer just our 16-byte algorithm.
  while ((length - i) >= sizeof(__m128i)) {
    __m128i samples = _mm_load_si128((__m128i*) (input + i));
#ifdef __SSSE3__
    samples = _mm_shuffle_epi8(samples, key);
#else
    __m128i swapleft = _mm_slli_epi16(samples, 8);
    __m128i swapright = _mm_srli_epi16(samples, 8);
    samples = _mm_or_si128(swapleft, swapright);
#endif
    _mm_store_si128((__m128i*) (output + i), samples);
    i += sizeof(samples);
  }

  // Finish last (8 byte) segment if one exists.
  if (length != i) {
    __m128i samples = _mm_loadl_epi64((__m128i*) (input + i));
#ifdef __SSSE3__
    samples = _mm_shuffle_epi8(samples, key);
#else
    __m128i swapleft = _mm_slli_epi16(samples, 8);
    __m128i swapright = _mm_srli_epi16(samples, 8);
    samples = _mm_or_si128(swapleft, swapright);
#endif
    _mm_storel_epi64((__m128i*) (output + i), samples);
  }

  return ret_buf;
#else
#error "Unimplemented byteswap_audio_buffer!"
#endif

}

