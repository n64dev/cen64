//
// ai/context.h: Sound card context management.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_AI_CONTEXT_H
#define CEN64_AI_CONTEXT_H
#include "common.h"
#include <AL/al.h>
#include <AL/alc.h>

struct cen64_ai_context {
  ALuint buffers[4];
  ALuint frequency;
  ALuint source;

  ALCdevice *dev;
  ALCcontext *ctx;
};

cen64_cold int ai_context_create(struct cen64_ai_context *context);
cen64_cold void ai_context_destroy(struct cen64_ai_context *context);

#endif

