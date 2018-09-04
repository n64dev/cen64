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
cen64_cold void arch_rsp_destroy(struct rsp *rsp);
cen64_cold int arch_rsp_init(struct rsp *rsp);

// Masks for AND/OR/XOR and NAND/NOR/NXOR.
extern const uint16_t rsp_vlogic_mask[2][8];

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
  return _mm_load_si128((__m128i *) src);
}

// Writes an operand back to memory.
static inline void rsp_vect_write_operand(uint16_t *dest, __m128i src) {
  _mm_store_si128((__m128i*) dest, src);
}

// Functions for reading/writing the accumulator.
//#if ((defined(__GNUC__) && !(defined(__clang__) || defined(__INTEL_COMPILER))) && defined(__x86_64))
#if 0
#define REGISTER_CACHING
register __m128i hr_acc_lo __asm__ ("xmm8");
register __m128i hr_acc_md __asm__ ("xmm9");
register __m128i hr_acc_hi __asm__ ("xmm10");

register __m128i hr_vcc_lo __asm__ ("xmm11");
register __m128i hr_vcc_hi __asm__ ("xmm12");
register __m128i hr_vco_lo __asm__ ("xmm13");
register __m128i hr_vco_hi __asm__ ("xmm14");
register __m128i hr_vce    __asm__ ("xmm15");

static inline __m128i read_acc_lo(const uint16_t *acc) {
  return hr_acc_lo;
}
static inline __m128i read_acc_md(const uint16_t *acc) {
  return hr_acc_md;
}
static inline __m128i read_acc_hi(const uint16_t *acc) {
  return hr_acc_hi;
}
static inline __m128i read_vcc_lo(const uint16_t *vcc) {
  return hr_vcc_lo;
}
static inline __m128i read_vcc_hi(const uint16_t *vcc) {
  return hr_vcc_hi;
}
static inline __m128i read_vco_lo(const uint16_t *vco) {
  return hr_vco_lo;
}
static inline __m128i read_vco_hi(const uint16_t *vco) {
  return hr_vco_hi;
}
static inline __m128i read_vce(const uint16_t *vce) {
  return hr_vce;
}
static inline void write_acc_lo(uint16_t *acc, __m128i acc_lo) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_acc_lo) : "x"(acc_lo));
}
static inline void write_acc_md(uint16_t *acc, __m128i acc_md) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_acc_md) : "x"(acc_md));
}
static inline void write_acc_hi(uint16_t *acc, __m128i acc_hi) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_acc_hi) : "x"(acc_hi));
}
static inline void write_vcc_lo(uint16_t *vcc, __m128i vcc_lo) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_vcc_lo) : "x"(vcc_lo));
}
static inline void write_vcc_hi(uint16_t *vcc, __m128i vcc_hi) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_vcc_hi) : "x"(vcc_hi));
}
static inline void write_vco_lo(uint16_t *vco, __m128i vco_lo) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_vco_lo) : "x"(vco_lo));
}
static inline void write_vco_hi(uint16_t *vco, __m128i vco_hi) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_vco_hi) : "x"(vco_hi));
}
static inline void write_vce(uint16_t *vce, __m128i vce_r) {
  __asm__ volatile("movdqa %1, %0\n\t" : "=x"(hr_vce) : "x"(vce_r));
}
#else
static inline __m128i read_acc_lo(const uint16_t *acc) {
  return rsp_vect_load_unshuffled_operand(acc + 16);
}
static inline __m128i read_acc_md(const uint16_t *acc) {
  return rsp_vect_load_unshuffled_operand(acc + 8);
}
static inline __m128i read_acc_hi(const uint16_t *acc) {
  return rsp_vect_load_unshuffled_operand(acc);
}
static inline __m128i read_vcc_lo(const uint16_t *vcc) {
  return rsp_vect_load_unshuffled_operand(vcc + 8);
}
static inline __m128i read_vcc_hi(const uint16_t *vcc) {
  return rsp_vect_load_unshuffled_operand(vcc);
}
static inline __m128i read_vco_lo(const uint16_t *vco) {
  return rsp_vect_load_unshuffled_operand(vco + 8);
}
static inline __m128i read_vco_hi(const uint16_t *vco) {
  return rsp_vect_load_unshuffled_operand(vco);
}
static inline __m128i read_vce(const uint16_t *vce) {
  return rsp_vect_load_unshuffled_operand(vce + 8);
}
static inline void write_acc_lo(uint16_t *acc, __m128i acc_lo) {
  rsp_vect_write_operand(acc + 16, acc_lo);
}
static inline void write_acc_md(uint16_t *acc, __m128i acc_md) {
  rsp_vect_write_operand(acc + 8, acc_md);
}
static inline void write_acc_hi(uint16_t *acc, __m128i acc_hi) {
  rsp_vect_write_operand(acc, acc_hi);
}
static inline void write_vcc_lo(uint16_t *vcc, __m128i vcc_lo) {
  rsp_vect_write_operand(vcc + 8, vcc_lo);
}
static inline void write_vcc_hi(uint16_t *vcc, __m128i vcc_hi) {
  rsp_vect_write_operand(vcc, vcc_hi);
}
static inline void write_vco_lo(uint16_t *vco, __m128i vco_lo) {
  rsp_vect_write_operand(vco + 8, vco_lo);
}
static inline void write_vco_hi(uint16_t *vco, __m128i vco_hi) {
  rsp_vect_write_operand(vco, vco_hi);
}
static inline void write_vce(uint16_t *vce, __m128i vce_r) {
  rsp_vect_write_operand(vce + 8, vce_r);
}
#endif

// Returns scalar bitmasks for VCO/VCC/VCE.
static inline int16_t rsp_get_flags(const uint16_t *flags) {
  return (int16_t) _mm_movemask_epi8(
    _mm_packs_epi16(
      _mm_load_si128((__m128i *) (flags + 8)),
      _mm_load_si128((__m128i *) (flags + 0))
    )
  );
}

void rsp_set_flags(uint16_t *flags, uint16_t rt);

// Zeroes out a vector register.
static inline __m128i rsp_vzero(void) {
  return _mm_setzero_si128();
}

// Load and store functions.
void rsp_vload_group1(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, __m128i reg, __m128i dqm);

void rsp_vload_group2(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, __m128i reg, __m128i dqm);

void rsp_vload_group4(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm);

void rsp_vstore_group1(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, __m128i reg, __m128i dqm);

void rsp_vstore_group2(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, __m128i reg, __m128i dqm);

void rsp_vstore_group4(struct rsp *rsp, uint32_t addr, unsigned element,
  uint16_t *regp, rsp_vect_t reg, rsp_vect_t dqm);

#include "arch/x86_64/rsp/clamp.h"
#include "arch/x86_64/rsp/vabs.h"
#include "arch/x86_64/rsp/vadd.h"
#include "arch/x86_64/rsp/vaddc.h"
#include "arch/x86_64/rsp/vand.h"
#include "arch/x86_64/rsp/vch.h"
#include "arch/x86_64/rsp/vcmp.h"
#include "arch/x86_64/rsp/vcl.h"
#include "arch/x86_64/rsp/vcr.h"
#include "arch/x86_64/rsp/vmac.h"
#include "arch/x86_64/rsp/vmrg.h"
#include "arch/x86_64/rsp/vmul.h"
#include "arch/x86_64/rsp/vmulh.h"
#include "arch/x86_64/rsp/vmull.h"
#include "arch/x86_64/rsp/vmulm.h"
#include "arch/x86_64/rsp/vmuln.h"
#include "arch/x86_64/rsp/vor.h"
#include "arch/x86_64/rsp/vsub.h"
#include "arch/x86_64/rsp/vsubc.h"
#include "arch/x86_64/rsp/vxor.h"

extern const uint16_t vdiv_mask_table[8][8];

__m128i rsp_vdivh(struct rsp *rsp,
  unsigned src, unsigned e, unsigned dest, unsigned de);

__m128i rsp_vmov(struct rsp *rsp,
  unsigned src, unsigned e, unsigned dest, rsp_vect_t vt_shuffle);

__m128i rsp_vrcp_vrsq(struct rsp *rsp, uint32_t iw, int dp,
  unsigned src, unsigned e, unsigned dest, unsigned de);

#endif

