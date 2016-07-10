//
// ai/context.c: Sound card context management.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "ai/context.h"

// Creates and initializes an audio context.
int ai_context_create(struct cen64_ai_context *context) {
  uint8_t buf[8];
  unsigned i;

  context->cur_frequency = 31985;

  if ((context->dev = alcOpenDevice(NULL)) == NULL) {
    printf("Failed to open the OpenAL device.\n");
    return 1;
  }

  if ((context->ctx = alcCreateContext(context->dev, NULL)) == NULL) {
    printf("Failed to create an OpenAL context.\n");
    alcCloseDevice(context->dev);
    return 1;
  }

  alcMakeContextCurrent(context->ctx);

  // Context/device is setup, create some buffers and a source.
  alGenBuffers(sizeof(context->buffers) / sizeof(*context->buffers),
    context->buffers);

  if (alGetError() != AL_NO_ERROR) {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context->ctx);
    alcCloseDevice(context->dev);
  }

  alGenSources(1, &context->source);

  if (alGetError() != AL_NO_ERROR) {
    alDeleteBuffers(sizeof(context->buffers) / sizeof(*context->buffers),
      context->buffers);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context->ctx);
    alcCloseDevice(context->dev);
  }

  // Queue/prime buffers, clear them to prevent pops.
  // Playing a little test also allows us to verify
  // that we (probably) won't get weird OpenAL errors
  // later on if things work now.
  memset(buf, 0x0, sizeof(buf));

  for (i = 0; i < sizeof(context->buffers) / sizeof(*context->buffers); i++)
    alBufferData(context->buffers[i], AL_FORMAT_STEREO16,
      buf, sizeof(buf), context->cur_frequency);

  context->unqueued_buffers = sizeof(context->buffers) /
    sizeof(*context->buffers);

  return 0;
}

// Destroys audio contexts made with ai_context_create.
void ai_context_destroy(struct cen64_ai_context *context) {
  alDeleteSources(1, &context->source);
  alDeleteBuffers(sizeof(context->buffers) / sizeof(*context->buffers),
    context->buffers);

  alcMakeContextCurrent(NULL);
  alcDestroyContext(context->ctx);
  alcCloseDevice(context->dev);
}

// Generate buffers for some given frequency.
int ai_switch_frequency(struct cen64_ai_context *context, ALint frequency) {
  alDeleteSources(1, &context->source);
  alDeleteBuffers(sizeof(context->buffers) / sizeof(*context->buffers),
    context->buffers);

  alGenBuffers(sizeof(context->buffers) / sizeof(*context->buffers),
    context->buffers);
  alGenSources(1, &context->source);

  context->cur_frequency = frequency;

  context->unqueued_buffers = sizeof(context->buffers) /
    sizeof(*context->buffers);

  return 0;
}

