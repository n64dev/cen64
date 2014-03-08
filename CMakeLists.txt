#
# CEN64: Cycle-Accurate Nintendo 64 Simulator.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

cmake_minimum_required(VERSION 2.6)
project(cen64)

if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_C_FLAGS "-Wall -Wextra -std=c99 -D_POSIX_SOURCE")

  set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -g3 -O0")
  set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Og")
endif ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/bus)
include_directories(${PROJECT_SOURCE_DIR}/pif)
include_directories(${PROJECT_SOURCE_DIR}/vr4300)

# Glob all the files together.
file(GLOB BUS_SOURCES ${PROJECT_SOURCE_DIR}/bus/*.c)
file(GLOB CEN64_SOURCES ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB PIF_SOURCES ${PROJECT_SOURCE_DIR}/pif/*.c)
file(GLOB VR4300_SOURCES ${PROJECT_SOURCE_DIR}/vr4300/*.c)

# Create the executable.
add_executable(cen64 ${CEN64_SOURCES} ${BUS_SOURCES} ${PIF_SOURCES} 
  ${VR4300_SOURCES})

