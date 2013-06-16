#  ============================================================================
#   Makefile for *NIX.
#
#   CEN64: Cycle-Accurate, Efficient Nintendo 64 Simulator.
#   Copyright (C) 2013, Tyler J. Stachecki.
#   All Rights Reserved.
#  ============================================================================
TARGET = cen64

# ============================================================================
#  A list of files to link into the library.
# ============================================================================
SOURCES := $(wildcard *.c)
OBJECTS = $(addprefix $(OBJECT_DIR)/, $(notdir $(SOURCES:.c=.o)))

LIBDIRS = -Laudio -Lbus -Lpif -Lrdram -Lrom -Lrsp -Lrdp -Lvideo -Lvr4300
LIBS = -laudio -lbus -lpif -lrdram -lrom -lrsp -lrdp -lvideo -lvr4300 \
  -lglfw -lm

# =============================================================================
#  Build variables and settings.
# =============================================================================
OBJECT_DIR=Objects

BLUE=$(shell tput setaf 4)
PURPLE=$(shell tput setaf 5)
TEXTRESET=$(shell tput sgr0)
YELLOW=$(shell tput setaf 3)

# ============================================================================
#  General build rules.
# ============================================================================
ECHO=/usr/bin/printf "%s\n"
MKDIR = /bin/mkdir -p

DOXYGEN = doxygen
STRIP = strip

CEN64_FLAGS = 
WARNINGS = -Wall -Wextra -pedantic -Wunsafe-loop-optimizations

COMMON_CFLAGS = $(CEN64_FLAGS) $(WARNINGS) -std=c99 -march=native -I.
COMMON_CXXFLAGS = $(CEN64_FLAGS) $(WARNINGS) -std=c++0x -march=native -I.
OPTIMIZATION_FLAGS = -flto -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -funsafe-loop-optimizations \
  -finline-limit=512

RELEASE_LDFLAGS = -Wl,-O1 -Wl,-as-needed -Wl,--gc-sections
RELEASE_CFLAGS = -DNDEBUG -O3 $(OPTIMIZATION_FLAGS)
DEBUG_CFLAGS = -DDEBUG -O0 -ggdb -g3

$(OBJECT_DIR)/%.o: %.c %.h Common.h
	@$(MKDIR) $(OBJECT_DIR)
	@$(ECHO) "$(BLUE)Compiling$(YELLOW): $(PURPLE)$(PREFIXDIR)$<$(TEXTRESET)"
	@$(CC) $(CFLAGS) $< -c -o $@

# ============================================================================
#  Targets.
# ============================================================================
all: CFLAGS = $(COMMON_CFLAGS) $(RELEASE_CFLAGS)
all: LDFLAGS = $(RELEASE_LDFLAGS)
all: TARGETTYPE = all
all: $(TARGET)-strip

all-static: CFLAGS = $(COMMON_CFLAGS) $(RELEASE_CFLAGS)
all-static: LDFLAGS = $(CFLAGS) $(RELEASE_LDFLAGS) -static
all-static: TARGETTYPE = all
all-static: $(TARGET)-strip

debug: CFLAGS = $(COMMON_CFLAGS) $(DEBUG_CFLAGS)
debug: TARGETTYPE = debug
debug: $(TARGET)

all-cpp: CFLAGS = $(COMMON_CXXFLAGS) $(RELEASE_CFLAGS)
all-cpp: LDFLAGS = $(CFLAGS) $(RELEASE_LDFLAGS)
all-cpp: TARGETTYPE = all-cpp
all-cpp: CC = $(CXX)
all-cpp: $(TARGET)-strip

all-cpp-static: CFLAGS = $(COMMON_CXXFLAGS) $(RELEASE_CFLAGS)
all-cpp-static: LDFLAGS = $(CFLAGS) $(RELEASE_LDFLAGS) -static
all-cpp-static: TARGETTYPE = all-cpp
all-cpp-static: CC = $(CXX)
all-cpp-static: $(TARGET)-strip

debug-cpp: CFLAGS = $(COMMON_CXXFLAGS) $(DEBUG_CFLAGS)
debug-cpp: TARGETTYPE = debug-cpp
debug-cpp: CC = $(CXX)
debug-cpp: $(TARGET)

$(TARGET)-strip: $(TARGET)
	@$(ECHO) "$(BLUE)Stripping$(YELLOW): $(PURPLE)$<$(TEXTRESET)"
	@$(STRIP) -s $<

$(TARGET): $(OBJECTS) libaudio libbus libpif \
  librdp librdram librom librsp libvideo libvr4300
	@$(ECHO) "$(BLUE)Linking$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJECTS) $(LIBS) -o $@

.PHONY: clean documentation inspect inspect-cpp
clean:
	@$(ECHO) "$(BLUE)Cleaning cen64...$(TEXTRESET)"
	@$(MAKE) -s -C audio clean
	@$(MAKE) -s -C bus clean
	@$(MAKE) -s -C pif clean
	@$(MAKE) -s -C rdram clean
	@$(MAKE) -s -C rdp clean
	@$(MAKE) -s -C rom clean
	@$(MAKE) -s -C rsp clean
	@$(MAKE) -s -C video clean
	@$(MAKE) -s -C vr4300 clean
	@$(RM) $(OBJECTS) $(TARGET)

inspect: $(TARGET)
	objdump -d $< | less

inspect-cpp: $(TARGET)
	objdump -d $< | c++filt | less

libaudio:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C audio $(TARGETTYPE)  PREFIXDIR=audio/

libbus:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C bus $(TARGETTYPE)  PREFIXDIR=bus/

libpif:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C pif $(TARGETTYPE)  PREFIXDIR=pif/

librdp:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C rdp $(TARGETTYPE)  PREFIXDIR=rdp/

librdram:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C rdram $(TARGETTYPE)  PREFIXDIR=rdram/

librom:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C rom $(TARGETTYPE)  PREFIXDIR=rom/

librsp:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C rsp $(TARGETTYPE) PREFIXDIR=rsp/

libvideo:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C video $(TARGETTYPE) PREFIXDIR=video/

libvr4300:
	@$(ECHO) "$(BLUE)Building$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(MAKE) -s -C vr4300 $(TARGETTYPE)  PREFIXDIR=vr4300/

