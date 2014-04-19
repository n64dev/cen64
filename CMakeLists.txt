#
# CEN64: Cycle-Accurate Nintendo 64 Simulator.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

cmake_minimum_required(VERSION 2.6)
project(cen64)

# If using GCC, configure it accordingly.
if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -std=c99 -D_POSIX_SOURCE")

  set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -g3 -O0")
  set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG -s")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -s")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Og")
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

  # Enable processor-specific optimizations?
  option(NATIVE_BUILD "Build binary for native architecture?" OFF)

  if (NATIVE_BUILD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
  endif (NATIVE_BUILD)

  # Enable link time optimization on recent versions.
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)

  if (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
    set(GCC_FLTO_FLAGS "-flto -fuse-linker-plugin -flto-partition=none -fdata-sections -ffunction-sections")

    # When compiling with -flto, need to pass CFLAGS to the linker.
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${GCC_FLTO_FLAGS}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${GCC_FLTO_FLAGS}")
  endif (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
endif ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")

# Print out MMIO register accesses?
option(DEBUG_MMIO_REGISTER_ACCESS "Print message on each MMIO register access?" OFF)

# Configure the common header.
configure_file(
  "${PROJECT_SOURCE_DIR}/common.h.in"
  "${PROJECT_BINARY_DIR}/common.h"
)

# Glob all the files together.
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB AI_SOURCES ${PROJECT_SOURCE_DIR}/ai/*.c)
file(GLOB BUS_SOURCES ${PROJECT_SOURCE_DIR}/bus/*.c)
file(GLOB CEN64_SOURCES ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB PI_SOURCES ${PROJECT_SOURCE_DIR}/pi/*.c)
file(GLOB RI_SOURCES ${PROJECT_SOURCE_DIR}/ri/*.c)
file(GLOB SI_SOURCES ${PROJECT_SOURCE_DIR}/si/*.c)
file(GLOB RDP_SOURCES ${PROJECT_SOURCE_DIR}/rdp/*.c)
file(GLOB RSP_SOURCES ${PROJECT_SOURCE_DIR}/rsp/*.c)
file(GLOB VI_SOURCES ${PROJECT_SOURCE_DIR}/vi/*.c)
file(GLOB VR4300_SOURCES ${PROJECT_SOURCE_DIR}/vr4300/*.c)

# Create the executable.
add_executable(cen64 ${CEN64_SOURCES}
  ${AI_SOURCES} ${PI_SOURCES} ${RI_SOURCES} ${SI_SOURCES} ${VI_SOURCES}
  ${BUS_SOURCES} ${RDP_SOURCES} ${RSP_SOURCES} ${VR4300_SOURCES})

