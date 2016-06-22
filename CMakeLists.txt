#
# CEN64: Cycle-Accurate Nintendo 64 Emulator.
# Copyright (C) 2015, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

cmake_minimum_required(VERSION 2.8)
project(cen64 C)

if(APPLE)
  set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})
  find_package(OpenGLXQuartz REQUIRED)
  # Needed for signal.h on OS X.
  add_definitions(-D_DARWIN_C_SOURCE)
else(APPLE)
  find_package(OpenGL REQUIRED)
endif(APPLE)

find_package(OpenAL REQUIRED)
find_package(Threads REQUIRED)

# If using GCC, configure it accordingly.
if (${CMAKE_C_COMPILER_ID} MATCHES GNU)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -std=c99")

  # Include architecture-specify machinery.
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE GCC_MACHINE)

  string(REGEX MATCH "([a-zA-Z0-9])+" GCC_MACHINE ${GCC_MACHINE})

  if (${GCC_MACHINE} MATCHES "x86.*" OR ${GCC_MACHINE} MATCHES "i.86.*")
    set(CEN64_ARCH_SUPPORT "SSE2" CACHE STRING "Architectural extension(s) to use")
    set_property(CACHE CEN64_ARCH_SUPPORT PROPERTY STRINGS SSE2 SSE3 SSSE3 SSE4.1 AVX)

    if (${CEN64_ARCH_SUPPORT} MATCHES "SSE2")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mssse3")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE4.1")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "AVX")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx")
    endif ()

    if (${GCC_MACHINE} MATCHES "i.86.*" OR ${GCC_MACHINE} MATCHES "x86.*")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffixed-xmm8 -ffixed-xmm9 -ffixed-xmm10")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffixed-xmm11 -ffixed-xmm12 -ffixed-xmm13 -ffixed-xmm14 -ffixed-xmm15")
    endif (${GCC_MACHINE} MATCHES "i.86.*" OR ${GCC_MACHINE} MATCHES "x86.*")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maccumulate-outgoing-args")

    set(CEN64_ARCH_DIR "x86_64")
    include_directories(${PROJECT_SOURCE_DIR}/os/unix/x86_64)
  endif (${GCC_MACHINE} MATCHES "x86.*" OR ${GCC_MACHINE} MATCHES "i.86.*")

  if (${GCC_MACHINE} STREQUAL "arm")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard -mfpu=neon")

    include_directories(${PROJECT_SOURCE_DIR}/arch/arm)
    include_directories(${PROJECT_SOURCE_DIR}/os/unix/arm)
  endif (${GCC_MACHINE} STREQUAL "arm")

  # Set architecture-independent flags.
  set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -g3 -O0")
  set(CMAKE_C_FLAGS_MINSIZEREL "-Os -ffast-math -DNDEBUG -s -fmerge-all-constants")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -ffast-math -DNDEBUG -ggdb3 -g3 -fmerge-all-constants")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG -fmerge-all-constants")
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

  # Enable link time optimization on recent versions.
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)

  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-s")

  if (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
    set(GCC_FLTO_FLAGS "-flto -fdata-sections -ffunction-sections")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${GCC_FLTO_FLAGS}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${GCC_FLTO_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -Wl,--gc-sections")
  endif (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)

  # Enable "unsafe" loop optimizations on recent versions.
  if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -funsafe-loop-optimizations")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -funsafe-loop-optimizations")
  endif (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)

  # Check for undefined behaviour when debugging.
  if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined")
  endif (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)

  # Use fat LTO objects.
  if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffat-lto-objects")
  endif (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
endif (${CMAKE_C_COMPILER_ID} MATCHES GNU)

# If using Clang, configure it accordingly.
if (${CMAKE_C_COMPILER_ID} MATCHES Clang)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -std=c99")

  # Include architecture-specify machinery.
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE CLANG_MACHINE)

  string(REGEX MATCH "([a-zA-Z0-9])+" CLANG_MACHINE ${CLANG_MACHINE})

  if (${CLANG_MACHINE} MATCHES "x86.*" OR ${CLANG_MACHINE} MATCHES "i.86.*")
    set(CEN64_ARCH_SUPPORT "SSE2" CACHE STRING "Architectural extension(s) to use")
    set_property(CACHE CEN64_ARCH_SUPPORT PROPERTY STRINGS SSE2 SSE3 SSSE3 SSE4.1 AVX Native)

    if (${CEN64_ARCH_SUPPORT} MATCHES "SSE2")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mssse3")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE4.1")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "AVX")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "Native")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
    endif ()

    set(CEN64_ARCH_DIR "x86_64")
    include_directories(${PROJECT_SOURCE_DIR}/os/unix/x86_64)
  endif (${CLANG_MACHINE} MATCHES "x86.*" OR ${CLANG_MACHINE} MATCHES "i.86.*")

  if (${CLANG_MACHINE} STREQUAL "arm")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard -mfpu=neon")

    set(CEN64_ARCH_DIR "arm")
    include_directories(${PROJECT_SOURCE_DIR}/os/unix/arm)
  endif (${CLANG_MACHINE} STREQUAL "arm")

  # Set architecture-independent flags.
  if (APPLE)
    set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -g3 -O0")
  else ()
    set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -g3 -O0 -fsanitize=undefined")
  endif ()
  set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -ggdb3 -g3")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-s")

  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
endif (${CMAKE_C_COMPILER_ID} MATCHES Clang)

# If using ICC, configure it accordingly.
if (${CMAKE_C_COMPILER_ID} MATCHES Intel)
  set(CMAKE_C_FLAGS "-Wall -Wno-unused-parameter -std=c99")

  set(CEN64_ARCH_SUPPORT "SSE2" CACHE STRING "Architectural extension(s) to use")
  set_property(CACHE CEN64_ARCH_SUPPORT PROPERTY STRINGS SSE2 SSE3 SSSE3 SSE4.1 AVX Native)

    if (${CEN64_ARCH_SUPPORT} MATCHES "SSE2")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -xSSE2")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -xSSSE3")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -xSSE3")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE4.1")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -xSSE4.1")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "AVX")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -xAVX")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "Native")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -xHost")
    endif ()

  set(CEN64_ARCH_DIR "x86_64")
  include_directories(${PROJECT_SOURCE_DIR}/os/unix/x86_64)

  # Set architecture-independent flags.
  set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
  set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG -ipo -ffunction-sections -fdata-sections")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -ipo -ffunction-sections -fdata-sections")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -ggdb3 -g3 -ipo -ffunction-sections -fdata-sections")
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
endif (${CMAKE_C_COMPILER_ID} MATCHES Intel)

# If using MSVC, configure it accordingly.
if (MSVC)
  set(CEN64_ARCH_SUPPORT "SSE2" CACHE STRING "Architectural extension(s) to use")
  set_property(CACHE CEN64_ARCH_SUPPORT PROPERTY STRINGS SSE2 SSE3 SSSE3 SSE4.1 AVX)

  if (${CEN64_ARCH_SUPPORT} MATCHES "SSE2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D__SSE2__")
  elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSSE3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D__SSE2__ /D__SSE3__ /D__SSSE3__")
  elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D__SSE2__ /D__SSE3__")
  elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE4.1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D__SSE2__ /D__SSE3__ /D__SSSE3__ /D__SSE4_1__")
  elseif (${CEN64_ARCH_SUPPORT} MATCHES "AVX")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D__SSE2__ /D__SSE3__ /D__SSSE3__ /D__SSE4_1__ /arch:AVX")
  endif ()

  enable_language(ASM_MASM) # Are you kidding me? Really?
  file(GLOB ASM_SOURCES ${PROJECT_SOURCE_DIR}/os/windows/x86_64/fpu/*.asm)

  set(CEN64_ARCH_DIR "x86_64")
  include_directories(${PROJECT_SOURCE_DIR}/os/windows/x86_64)
endif (MSVC)

# Print out MMIO register accesses?
option(DEBUG_MMIO_REGISTER_ACCESS "Print message on each MMIO register access?" OFF)

# Use VR4300's busy-wait-detection feature?
option(VR4300_BUSY_WAIT_DETECTION "Detect and special case VR4300 busy wait loops?" OFF)

# Glob all the files together.
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/arch/${CEN64_ARCH_DIR})
include_directories(${PROJECT_SOURCE_DIR}/os/common)
include_directories(${OPENAL_INCLUDE_DIR})

#
# Create lists of files.
#
set(AI_SOURCES
  ${PROJECT_SOURCE_DIR}/ai/context.c
  ${PROJECT_SOURCE_DIR}/ai/controller.c
)

set(ARCH_X86_64_SOURCES
  ${PROJECT_SOURCE_DIR}/arch/x86_64/tlb/tlb.c
  ${PROJECT_SOURCE_DIR}/arch/x86_64/rsp/vrcpsq.c
  ${PROJECT_SOURCE_DIR}/arch/x86_64/rsp/vmov.c
  ${PROJECT_SOURCE_DIR}/arch/x86_64/rsp/vdivh.c
  ${PROJECT_SOURCE_DIR}/arch/x86_64/rsp/rsp.c
  ${PROJECT_SOURCE_DIR}/arch/x86_64/rsp/vrsq.c
)

set(BUS_SOURCES
  ${PROJECT_SOURCE_DIR}/bus/controller.c
  ${PROJECT_SOURCE_DIR}/bus/memorymap.c
)

set(COMMON_SOURCES
  ${PROJECT_SOURCE_DIR}/common/debug.c
  ${PROJECT_SOURCE_DIR}/common/one_hot.c
  ${PROJECT_SOURCE_DIR}/common/reciprocal.c
)

set(DD_SOURCES
  ${PROJECT_SOURCE_DIR}/dd/controller.c
)

set(DEVICE_SOURCES
  ${PROJECT_SOURCE_DIR}/cen64.c
  ${PROJECT_SOURCE_DIR}/device/cart_db.c
  ${PROJECT_SOURCE_DIR}/device/device.c
  ${PROJECT_SOURCE_DIR}/device/netapi.c
  ${PROJECT_SOURCE_DIR}/device/options.c
  ${PROJECT_SOURCE_DIR}/device/sha1.c
)

set(OS_SOURCES
  ${PROJECT_SOURCE_DIR}/os/common/gl_hints.c
  ${PROJECT_SOURCE_DIR}/os/common/input.c
)

set(OS_POSIX_SOURCES
  ${PROJECT_SOURCE_DIR}/os/posix/alloc.c
  ${PROJECT_SOURCE_DIR}/os/posix/local_time.c
  ${PROJECT_SOURCE_DIR}/os/posix/main.c
  ${PROJECT_SOURCE_DIR}/os/posix/rom_file.c
  ${PROJECT_SOURCE_DIR}/os/posix/save_file.c
  ${PROJECT_SOURCE_DIR}/os/posix/timer.c
)

set(OS_WINAPI_SOURCES
  ${PROJECT_SOURCE_DIR}/os/winapi/alloc.c
  ${PROJECT_SOURCE_DIR}/os/winapi/gl_config.c
  ${PROJECT_SOURCE_DIR}/os/winapi/gl_window.c
  ${PROJECT_SOURCE_DIR}/os/winapi/local_time.c
  ${PROJECT_SOURCE_DIR}/os/winapi/main.c
  ${PROJECT_SOURCE_DIR}/os/winapi/rom_file.c
  ${PROJECT_SOURCE_DIR}/os/winapi/save_file.c
  ${PROJECT_SOURCE_DIR}/os/winapi/timer.c
)

set(OS_X11_SOURCES
  ${PROJECT_SOURCE_DIR}/os/x11/gl_config.c
  ${PROJECT_SOURCE_DIR}/os/x11/gl_window.c
)

set(PI_SOURCES
  ${PROJECT_SOURCE_DIR}/pi/controller.c
)

set(RDP_SOURCES
  ${PROJECT_SOURCE_DIR}/rdp/cpu.c
  ${PROJECT_SOURCE_DIR}/rdp/interface.c
  ${PROJECT_SOURCE_DIR}/rdp/n64video.c
)

set(RI_SOURCES
  ${PROJECT_SOURCE_DIR}/ri/controller.c
)

set(RSP_SOURCES
  ${PROJECT_SOURCE_DIR}/rsp/cp0.c
  ${PROJECT_SOURCE_DIR}/rsp/cp2.c
  ${PROJECT_SOURCE_DIR}/rsp/cpu.c
  ${PROJECT_SOURCE_DIR}/rsp/decoder.c
  ${PROJECT_SOURCE_DIR}/rsp/functions.c
  ${PROJECT_SOURCE_DIR}/rsp/interface.c
  ${PROJECT_SOURCE_DIR}/rsp/opcodes.c
  ${PROJECT_SOURCE_DIR}/rsp/pipeline.c
  ${PROJECT_SOURCE_DIR}/rsp/vfunctions.c
)

set(SI_SOURCES
  ${PROJECT_SOURCE_DIR}/si/cic.c
  ${PROJECT_SOURCE_DIR}/si/controller.c
  ${PROJECT_SOURCE_DIR}/si/pak.c
  ${PROJECT_SOURCE_DIR}/si/pak_transfer.c
  ${PROJECT_SOURCE_DIR}/si/gb.c
  ${PROJECT_SOURCE_DIR}/si/rtc.c
)

set(VI_SOURCES
  ${PROJECT_SOURCE_DIR}/vi/controller.c
  ${PROJECT_SOURCE_DIR}/vi/render.c
  ${PROJECT_SOURCE_DIR}/vi/window.c
)

set(VR4300_SOURCES
  ${PROJECT_SOURCE_DIR}/vr4300/cp0.c
  ${PROJECT_SOURCE_DIR}/vr4300/cp1.c
  ${PROJECT_SOURCE_DIR}/vr4300/cpu.c
  ${PROJECT_SOURCE_DIR}/vr4300/dcache.c
  ${PROJECT_SOURCE_DIR}/vr4300/decoder.c
  ${PROJECT_SOURCE_DIR}/vr4300/fault.c
  ${PROJECT_SOURCE_DIR}/vr4300/functions.c
  ${PROJECT_SOURCE_DIR}/vr4300/icache.c
  ${PROJECT_SOURCE_DIR}/vr4300/interface.c
  ${PROJECT_SOURCE_DIR}/vr4300/opcodes.c
  ${PROJECT_SOURCE_DIR}/vr4300/pipeline.c
  ${PROJECT_SOURCE_DIR}/vr4300/segment.c
)

#
# Build OS_SOURCES list.
#
if (DEFINED WIN32)
  include_directories(${PROJECT_SOURCE_DIR}/os/winapi)
  set(EXTRA_OS_EXE WIN32)
  set(EXTRA_OS_LIBS mingw32 opengl32 winmm ws2_32)

  list(APPEND OS_SOURCES
    ${OS_COMMON_SOURCES}
    ${OS_WINAPI_SOURCES}
  )
else ()
  include_directories(${PROJECT_SOURCE_DIR}/os/posix)
  include_directories(${PROJECT_SOURCE_DIR}/os/x11)
  include_directories(${X11_xf86vmode_INCLUDE_PATH})

  list(APPEND OS_SOURCES
    ${OS_COMMON_SOURCES}
    ${OS_POSIX_SOURCES}
    ${OS_X11_SOURCES}
  )
endif (DEFINED WIN32)

#
# Glob all the files together.
#
if (DEFINED UNIX)
  find_package(X11 REQUIRED)

  if (${CMAKE_C_COMPILER_ID} MATCHES GNU OR ${CMAKE_C_COMPILER_ID} MATCHES Clang OR ${CMAKE_C_COMPILER_ID} MATCHES Intel)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_POSIX_C_SOURCE=200112L -D_BSD_SOURCE")
  endif (${CMAKE_C_COMPILER_ID} MATCHES GNU OR ${CMAKE_C_COMPILER_ID} MATCHES Clang OR ${CMAKE_C_COMPILER_ID} MATCHES Intel)
endif (DEFINED UNIX)

#
# Configure the common header.
#
configure_file(
  "${PROJECT_SOURCE_DIR}/common.h.in"
  "${PROJECT_BINARY_DIR}/common.h"
)

#
# Create the executable.
#
if (NOT MSVC)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/rdp/n64video.c PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)
endif (NOT MSVC)

add_executable(cen64
  ${EXTRA_OS_EXE}
  ${AI_SOURCES}
  ${ARCH_X86_64_SOURCES}
  ${BUS_SOURCES}
  ${COMMON_SOURCES}
  ${DD_SOURCES}
  ${DEVICE_SOURCES}
  ${OS_SOURCES}
  ${PI_SOURCES}
  ${RDP_SOURCES}
  ${RI_SOURCES}
  ${RSP_SOURCES}
  ${SI_SOURCES}
  ${VI_SOURCES}
  ${VR4300_SOURCES}
)

target_link_libraries(cen64
	${EXTRA_OS_LIBS}
  ${OPENAL_LIBRARY}
  ${OPENGL_gl_LIBRARY}
  ${X11_X11_LIB}
  ${CMAKE_THREAD_LIBS_INIT}
)

