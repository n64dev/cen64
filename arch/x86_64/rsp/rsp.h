//
// arch/x86_64/rsp/rsp.h
//
// Extern declarations for host RSP functions.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __arch_rsp_h__
#define __arch_rsp_h__
#include "common.h"

#ifdef __SSE4_2__
#include <nmmintrin.h>
#elif defined(__SSE4_1__)
#include <smmintrin.h>
#elif defined(__SSSE3__)
#include <tmmintrin.h>
#elif defined(__SSE3__)
#include <pmmintrin.h>
#else
#include <emmintrin.h>
#endif

struct rsp;
typedef __m128i rsp_vect_t;

// Gives the architecture backend a chance to initialize the RSP.
void arch_rsp_destroy(struct rsp *rsp);
int arch_rsp_init(struct rsp *rsp);

// Loads and shuffles a 16x8 vector according to element.
#ifdef __SSSE3__
extern const uint16_t shuffle_keys[16][8];

static inline __m128i rsp_vect_load_and_shuffle_operand(
  const uint16_t *src, unsigned element) {
  __m128i operand = _mm_load_si128((__m128i*) src);
  __m128i key = _mm_load_si128((__m128i*) shuffle_keys[element]);

  return _mm_shuffle_epi8(operand, key);
}
#else
__m128i rsp_vect_load_and_shuffle_operand(
  const uint16_t *src, unsigned element);
#endif

// Loads a vector without shuffling its elements.
static inline __m128i rsp_vect_load_unshuffled_operand(const uint16_t *src) {
  return _mm_load_si128((__m128i*) src);
}

// Writes an operand back to memory.
static inline void rsp_vect_write_operand(uint16_t *dest, __m128i src) {
  _mm_store_si128((__m128i*) dest, src);
}

// Functions for reading/writing the accumulator.
#if ((defined(__GNUC__) && !(defined(__clang__) || defined(__INTEL_COMPILER))) \
  && (defined(__i386__) || defined(__x86_64)))

#ifdef __i386__
register __m128i hr_acc_lo __asm__ ("xmm5");
register __m128i hr_acc_md __asm__ ("xmm6");
register __m128i hr_acc_hi __asm__ ("xmm7");
#else
register __m128i hr_acc_lo __asm__ ("xmm13");
register __m128i hr_acc_md __asm__ ("xmm14");
register __m128i hr_acc_hi __asm__ ("xmm15");
#endif

static inline void read_acc_lo(const uint16_t *acc, __m128i *acc_lo) {
  *acc_lo = hr_acc_lo;
}
static inline void read_acc_md(const uint16_t *acc, __m128i *acc_md) {
  *acc_md = hr_acc_md;
}
static inline void read_acc_hi(const uint16_t *acc, __m128i *acc_hi) {
  *acc_hi = hr_acc_hi;
}
static inline void write_acc_lo(uint16_t *acc, __m128i acc_lo) {
  hr_acc_lo = acc_lo;
}
static inline void write_acc_md(uint16_t *acc, __m128i acc_md) {
  hr_acc_md = acc_md;
}
static inline void write_acc_hi(uint16_t *acc, __m128i acc_hi) {
  hr_acc_hi = acc_hi;
}
#else
static inline void read_acc_lo(const uint16_t *acc, __m128i *acc_lo) {
  *acc_lo = rsp_vect_load_unshuffled_operand(acc + 16);
}
static inline void read_acc_md(const uint16_t *acc, __m128i *acc_md) {
  *acc_md = rsp_vect_load_unshuffled_operand(acc + 8);
}
static inline void read_acc_hi(const uint16_t *acc, __m128i *acc_hi) {
  *acc_hi = rsp_vect_load_unshuffled_operand(acc + 0);
}
static inline void write_acc_lo(uint16_t *acc, __m128i acc_lo) {
  rsp_vect_write_operand(acc + 16, acc_lo);
}
static inline void write_acc_md(uint16_t *acc, __m128i acc_md) {
  rsp_vect_write_operand(acc + 8, acc_md);
}
static inline void write_acc_hi(uint16_t *acc, __m128i acc_hi) {
  rsp_vect_write_operand(acc + 0, acc_hi);
}
#endif

// Zeroes out a register.
static inline __m128i rsp_vzero(void) {
  return _mm_setzero_si128();
}

// Sets all bits in a vector.
static inline __m128i rsp_vset(void) {
  __m128i junk;

  // GCC will try to `pxor xmm,xmm` as a peephole optimization to the
  // processor if we try to init junk using _mm_setzero_si128, don't
  // explicitly provide a value, initialize it to itself, or whatever
  // else. Skip the pxor instruction by just telling GCC to give us
  // back whatever's in the register.
#ifdef __GNUC__
  __asm__ __volatile__("" : "=x" (junk));
#else
  junk = _mm_setzero_si128();
#endif
  return _mm_cmpeq_epi32(junk, junk);
}

// Load and store aligner.
__m128i rsp_vload_dmem(struct rsp *rsp,
  uint32_t addr, unsigned element, __m128i reg, __m128i dqm);
void rsp_vstore_dmem(struct rsp *rsp,
  uint32_t addr, unsigned element, __m128i reg, __m128i dqm);

#include "arch/x86_64/rsp/clamp.h"

#include "arch/x86_64/rsp/vadd.h"
#include "arch/x86_64/rsp/vand.h"
#include "arch/x86_64/rsp/vmadh.h"
#include "arch/x86_64/rsp/vmadl.h"
#include "arch/x86_64/rsp/vmadm.h"
#include "arch/x86_64/rsp/vmadn.h"
#include "arch/x86_64/rsp/vmudh.h"
#include "arch/x86_64/rsp/vmudl.h"
#include "arch/x86_64/rsp/vmudm.h"
#include "arch/x86_64/rsp/vmudn.h"
#include "arch/x86_64/rsp/vnand.h"
#include "arch/x86_64/rsp/vnor.h"
#include "arch/x86_64/rsp/vor.h"
#include "arch/x86_64/rsp/vnxor.h"
#include "arch/x86_64/rsp/vsub.h"
#include "arch/x86_64/rsp/vxor.h"

#endif

