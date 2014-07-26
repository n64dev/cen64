#
# CEN64: Cycle-Accurate Nintendo 64 Simulator.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

cmake_minimum_required(VERSION 2.6)
project(cen64)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# If using GCC, configure it accordingly.
if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -std=c99")

  # Enable processor-specific optimizations?
  option(NATIVE_BUILD "Build binary for native architecture?" OFF)

  if (NATIVE_BUILD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
  endif (NATIVE_BUILD)

  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -ggdb3 -g3 -O0")
  set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS} -Os -DNDEBUG -s")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2 -DNDEBUG -s")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS} -Og")
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

  # Enable link time optimization on recent versions.
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)

  if (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
    set(GCC_FLTO_FLAGS "-flto -fuse-linker-plugin -flto-partition=none -fdata-sections -ffunction-sections")

    # When compiling with -flto, need to pass CFLAGS to the linker.
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${GCC_FLTO_FLAGS}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${GCC_FLTO_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS_RELEASE} "-Wl,--gc-sections")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} "-Wl,--gc-sections")
  endif (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)

  include_directories(${PROJECT_SOURCE_DIR}/os/unix/fpu/x86_64)
endif ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")

# If using Clang, configure it accordingly.
if ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -std=c99")

  # Enable processor-specific optimizations?
  option(NATIVE_BUILD "Build binary for native architecture?" OFF)

  if (NATIVE_BUILD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
  endif (NATIVE_BUILD)

  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -ggdb3 -g3 -O0 -fsanitize=undefined")
  set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS} -Os -DNDEBUG -flto")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -DNDEBUG -flto")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS} -Og")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_C_FLAGS} -s")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS} -s")

  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

  include_directories(${PROJECT_SOURCE_DIR}/os/unix/fpu/x86_64)
endif ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")

# If using MSVC, configure it accordingly.
if (MSVC)
  enable_language(ASM_MASM) # Are you kidding me? Really?

  file(GLOB ASM_SOURCES ${PROJECT_SOURCE_DIR}/os/windows/fpu/x86_64/*.asm)
  include_directories(${PROJECT_SOURCE_DIR}/os/windows/fpu/x86_64)
endif (MSVC)

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
file(GLOB COMMON_SOURCES ${PROJECT_SOURCE_DIR}/common/*.c)
file(GLOB CEN64_SOURCES ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB PI_SOURCES ${PROJECT_SOURCE_DIR}/pi/*.c)
file(GLOB RI_SOURCES ${PROJECT_SOURCE_DIR}/ri/*.c)
file(GLOB SI_SOURCES ${PROJECT_SOURCE_DIR}/si/*.c)
file(GLOB RDP_SOURCES ${PROJECT_SOURCE_DIR}/rdp/*.c)
file(GLOB RSP_SOURCES ${PROJECT_SOURCE_DIR}/rsp/*.c)
file(GLOB VI_SOURCES ${PROJECT_SOURCE_DIR}/vi/*.c)
file(GLOB VR4300_SOURCES ${PROJECT_SOURCE_DIR}/vr4300/*.c)

#
# Glob all the files together.
#
if (DEFINED UNIX)
  find_package(X11 REQUIRED)

  if (${CMAKE_C_COMPILER_ID} MATCHES GNU OR ${CMAKE_C_COMPILER_ID} MATCHES Clang)
    set(CMAKE_C_FLAGS "-D_POSIX_SOURCE")
  endif (${CMAKE_C_COMPILER_ID} MATCHES GNU OR ${CMAKE_C_COMPILER_ID} MATCHES Clang)

  include_directories(${X11_xf86vmode_INCLUDE_PATH})
  file(GLOB OS_SOURCES ${PROJECT_SOURCE_DIR}/os/unix/*.c)
  set(EXTRA_OS_LIBS m ${X11_X11_LIB} ${X11_Xxf86vm_LIB})
  set(EXTRA_OS_EXE "")
endif (DEFINED UNIX)

if (DEFINED WIN32)
  file(GLOB OS_SOURCES ${PROJECT_SOURCE_DIR}/os/windows/*.c)
  set(EXTRA_OS_LIBS ws2_32 Opengl32)
  set(EXTRA_OS_EXE WIN32)
endif (DEFINED WIN32)

# Create the executable.
add_executable(cen64 ${EXTRA_OS_EXE} ${ASM_SOURCES} ${OS_SOURCES}
  ${COMMON_SOURCES} ${CEN64_SOURCES} ${AI_SOURCES} ${BUS_SOURCES} ${PI_SOURCES}
  ${RDP_SOURCES} ${RSP_SOURCES} ${RI_SOURCES} ${SI_SOURCES} ${VI_SOURCES}
  ${VR4300_SOURCES})

target_link_libraries(cen64 ${EXTRA_OS_LIBS}
  ${OPENGL_gl_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

