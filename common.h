//
// common.h: Common definitions and such.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __common_h__
#define __common_h__

#ifdef _MSC_VER
#define inline _inline
#endif

#ifndef __cplusplus
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#else
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#endif

#ifndef _MSC_VER
#ifndef __cplusplus
#include <stdbool.h>
#else
#include <cstdbool>
#endif

#else
typedef char bool;
#define false 0
#define true 1
#endif

#ifndef NDEBUG
#ifndef __cplusplus
#include <stdio.h>
#else
#include <cstdio>
#endif
#endif

#define CACHE_LINE_SIZE 64

// Define cen64_align().
#ifdef _MSC_VER
#define cen64_align(decl, value) __declspec(align(value)) decl

#elif (defined __GNUC__)
#define cen64_align(decl, value) decl __attribute__ ((aligned(value)))

#else
#define cen64_align(decl, value) decl value
#endif

// Define likely()/unlikely().
#ifdef __GNUC__
#define likely(expr) __builtin_expect(!!(expr), !0)
#define unlikely(expr) __builtin_expect(!!(expr), 0)

#else
#define likely(expr) expr
#define unlikely(expr) expr
#endif

// Define unused().
#ifdef __GNUC__
#define unused(decl) __attribute__((unused)) decl
#else
#define unused(decl) decl
#endif

#endif

