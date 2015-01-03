#
# CEN64: Cycle-Accurate Nintendo 64 Simulator.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

cmake_minimum_required(VERSION 2.8)
project(cen64)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# If using GCC, configure it accordingly.
if (${CMAKE_C_COMPILER_ID} MATCHES GNU)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -std=c99")
  set(CEN64_DEV_FLAG "-DCEN64_DEVFEATURES")

  # Include architecture-specify machinery.
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE GCC_MACHINE)

  string(REGEX MATCH "([a-zA-Z0-9])+" GCC_MACHINE ${GCC_MACHINE})

  if (${GCC_MACHINE} MATCHES "x86.*" OR ${GCC_MACHINE} MATCHES "i.86.*")
    enable_language(ASM-ATT)

    set(CEN64_ARCH_SUPPORT "SSE2" CACHE STRING "Architectural extension(s) to use")
    set_property(CACHE CEN64_ARCH_SUPPORT PROPERTY STRINGS SSE2 SSE3 SSSE3 SSE4.1 AVX)

    if (${CEN64_ARCH_SUPPORT} MATCHES "SSE2")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
      set(CMAKE_ASM-ATT_FLAGS "${CMAKE_ASM-ATT_FLAGS} -march=sse2 --defsym __SSE2__=1")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mssse3")
      set(CMAKE_ASM-ATT_FLAGS "${CMAKE_ASM-ATT_FLAGS} -march=ssse3 --defsym __SSSE3__=1 --defsym __SSE3__=1")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE3")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3")
      set(CMAKE_ASM-ATT_FLAGS "${CMAKE_ASM-ATT_FLAGS} -march=sse3 --defsym __SSE3__=1 --defsym __SSE2__=1")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "SSE4.1")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4")
      set(CMAKE_ASM-ATT_FLAGS "${CMAKE_ASM-ATT_FLAGS} -march=sse4.1 --defsym __SSE4_1__=1 --defsym __SSSE3__=1 --defsym __SSE3__=1")
    elseif (${CEN64_ARCH_SUPPORT} MATCHES "AVX")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx")
      set(CMAKE_ASM-ATT_FLAGS "${CMAKE_ASM-ATT_FLAGS} -march=avx --defsym __AVX__=1")
    endif ()

#    if (${GCC_MACHINE} MATCHES "i.86.*" OR ${GCC_MACHINE} MATCHES "x86.*")
#      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffixed-xmm8 -ffixed-xmm9 -ffixed-xmm10")
#      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffixed-xmm11 -ffixed-xmm12 -ffixed-xmm13 -ffixed-xmm14 -ffixed-xmm15")
#    endif (${GCC_MACHINE} MATCHES "i.86.*" OR ${GCC_MACHINE} MATCHES "x86.*")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maccumulate-outgoing-args")

    set(CEN64_ARCH_DIR "x86_64")
    include_directories(${PROJECT_SOURCE_DIR}/os/unix/x86_64)

#    file(GLOB ASM_SOURCES ${PROJECT_SOURCE_DIR}/arch/x86_64/rsp/gcc/*.s)
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
  set(CEN64_DEV_FLAG "-DCEN64_DEVFEATURES")

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
  set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -g3 -O0 -fsanitize=undefined")
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
  set(CEN64_DEV_FLAG "-DCEN64_DEVFEATURES")

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
  set(CEN64_DEV_FLAG "/DCEN64_DEVFEATURES")

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

file(GLOB AI_SOURCES ${PROJECT_SOURCE_DIR}/ai/*.c)
file(GLOB BUS_SOURCES ${PROJECT_SOURCE_DIR}/bus/*.c)
file(GLOB COMMON_SOURCES ${PROJECT_SOURCE_DIR}/common/*.c)
file(GLOB OS_COMMON_SOURCES ${PROJECT_SOURCE_DIR}/os/*.c)
file(GLOB PI_SOURCES ${PROJECT_SOURCE_DIR}/pi/*.c)
file(GLOB RI_SOURCES ${PROJECT_SOURCE_DIR}/ri/*.c)
file(GLOB SI_SOURCES ${PROJECT_SOURCE_DIR}/si/*.c)
file(GLOB RDP_SOURCES ${PROJECT_SOURCE_DIR}/rdp/*.c)
file(GLOB RSP_SOURCES ${PROJECT_SOURCE_DIR}/rsp/*.c)
file(GLOB VI_SOURCES ${PROJECT_SOURCE_DIR}/vi/*.c)
file(GLOB VR4300_SOURCES ${PROJECT_SOURCE_DIR}/vr4300/*.c)
file(GLOB ARCH_FPU_SOURCES arch/${CEN64_ARCH_DIR}/fpu/*.c)
file(GLOB ARCH_RSP_SOURCES arch/${CEN64_ARCH_DIR}/rsp/*.c)
file(GLOB ARCH_TLB_SOURCES arch/${CEN64_ARCH_DIR}/tlb/*.c)

#
# Glob all the files together.
#
if (DEFINED UNIX)
  find_package(X11 REQUIRED)

  if (${CMAKE_C_COMPILER_ID} MATCHES GNU OR ${CMAKE_C_COMPILER_ID} MATCHES Clang OR ${CMAKE_C_COMPILER_ID} MATCHES Intel)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_POSIX_C_SOURCE=200112L -D_BSD_SOURCE")
  endif (${CMAKE_C_COMPILER_ID} MATCHES GNU OR ${CMAKE_C_COMPILER_ID} MATCHES Clang OR ${CMAKE_C_COMPILER_ID} MATCHES Intel)

  include_directories(${X11_xf86vmode_INCLUDE_PATH})
  file(GLOB OS_SOURCES ${PROJECT_SOURCE_DIR}/os/unix/*.c)
  set(EXTRA_OS_LIBS ${X11_X11_LIB} ${X11_Xxf86vm_LIB})
  set(EXTRA_OS_EXE "")
endif (DEFINED UNIX)

if (DEFINED WIN32)
  file(GLOB OS_SOURCES ${PROJECT_SOURCE_DIR}/os/windows/*.c)
  set(EXTRA_OS_LIBS ws2_32 winmm opengl32)
  set(EXTRA_OS_EXE WIN32)
endif (DEFINED WIN32)

# Configure the common header.
configure_file(
  "${PROJECT_SOURCE_DIR}/common.h.in"
  "${PROJECT_BINARY_DIR}/common.h"
)

# Create static libraries.
add_library(cen64arch STATIC ${ASM_SOURCES}
  ${ARCH_FPU_SOURCES}
  ${ARCH_RSP_SOURCES}
  ${ARCH_TLB_SOURCES}
)

add_library(cen64os STATIC "${PROJECT_SOURCE_DIR}/cen64.c"
  ${COMMON_SOURCES}
  ${OS_COMMON_SOURCES}
  ${OS_SOURCES}
)

add_library(cen64ai STATIC ${AI_SOURCES})
add_library(cen64bus STATIC ${BUS_SOURCES})
add_library(cen64pi STATIC ${PI_SOURCES})
add_library(cen64rdp STATIC ${RDP_SOURCES})
add_library(cen64ri STATIC ${RI_SOURCES})
add_library(cen64si STATIC ${SI_SOURCES})
add_library(cen64rsp STATIC ${RSP_SOURCES})
add_library(cen64vi STATIC ${VI_SOURCES})
add_library(cen64vr4300 STATIC ${VR4300_SOURCES})

# Create the executables.
set_source_files_properties(${PROJECT_BINARY_DIR}/device.c PROPERTIES COMPILE_FLAGS ${CEN64_DEV_FLAG})

add_executable(cen64 ${EXTRA_OS_EXE} ${PROJECT_SOURCE_DIR}/device/device_fast.c)
add_executable(cen64dev ${EXTRA_OS_EXE} ${PROJECT_SOURCE_DIR}/device/device_dev.c)

target_link_libraries(cen64
	cen64ai cen64bus cen64pi cen64rdp cen64ri cen64rsp cen64si cen64vr4300 cen64arch cen64os cen64vi
	${EXTRA_OS_LIBS} ${OPENGL_gl_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(cen64dev
	cen64ai cen64bus cen64pi cen64rdp cen64ri cen64rsp cen64si cen64vr4300 cen64arch cen64os cen64vi
	${EXTRA_OS_LIBS} ${OPENGL_gl_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

