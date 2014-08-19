//
// arch/arm/rsp/rsp.h
//
// Extern declarations for host RSP functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __arch_rsp_h__
#define __arch_rsp_h__
#include "common.h"
#include <arm_neon.h>

extern const uint8_t shuffle_keys[16][16];

// Loads and shuffles a 16x8 vector according to element.
static inline uint16x8_t load_and_shuffle_operand(
  const uint8_t *src, unsigned element) {
  const uint8x16_t keys = vld1q_u8(shuffle_keys[element]);
  const uint8x8x2_t packed_operand = vld2_u8(src);

  uint8x8_t operand_lo_8x8 = vtbl2_u8(packed_operand, vget_low_u8(keys));
  uint8x8_t operand_hi_8x8 = vtbl2_u8(packed_operand, vget_high_u8(keys));
  uint8x16_t operand = vcombine_u8(operand_lo_8x8, operand_hi_8x8);

  return vreinterpretq_u16_u8(operand);
}

// Loads a vector without shuffling its elements.
static inline uint16x8_t load_unshuffled_operand(const uint8_t *src) {
  return vreinterpretq_u16_u8(vld1q_u8(src));
}

// Writes an operand back to memory.
static inline void write_operand(uint8_t *dest, uint16x8_t src) {
  vst1q_u8(dest, vreinterpretq_u8_u16(src));
}

#endif

