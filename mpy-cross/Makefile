include ../py/mkenv.mk

# define main target
# CIRCUITPY-CHANGE
PROG ?= mpy-cross

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# OS name, for simple autoconfig
UNAME_S := $(shell uname -s)

# include py core make definitions
include $(TOP)/py/py.mk

INC += -I.
INC += -I$(BUILD)
INC += -I$(TOP)

# compiler settings
CWARN = -Wall -Werror
CWARN += -Wextra -Wno-unused-parameter -Wpointer-arith
CFLAGS += $(INC) $(CWARN) -std=gnu99 $(COPT) $(CFLAGS_EXTRA)
CFLAGS += -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables
# CIRCUITPY-CHANGE
CFLAGS += -DCIRCUITPY

# Build a static executable.
# Useful for builds that must run on multiple operating system versions. Used for published mpy-cross versions.
ifdef STATIC_BUILD
CFLAGS += -static -static-libgcc -static-libstdc++
endif

# Debugging/Optimization
ifdef DEBUG
CFLAGS += -g
COPT = -O0
else
COPT = -Os #-DNDEBUG
endif

# On OSX, 'gcc' is a symlink to clang unless a real gcc is installed.
# The unix port of MicroPython on OSX must be compiled with clang,
# while cross-compile ports require gcc, so we test here for OSX and
# if necessary override the value of 'CC' set in py/mkenv.mk
ifeq ($(UNAME_S),Darwin)
CC = clang
# Use clang syntax for map file
LDFLAGS_ARCH = -Wl,-map,$@.map -Wl,-dead_strip
else
# Use gcc syntax for map file
LDFLAGS_ARCH = -Wl,-Map=$@.map,--cref -Wl,--gc-sections
endif
LDFLAGS += $(LDFLAGS_MOD) $(LDFLAGS_ARCH) -lm $(LDFLAGS_EXTRA)

ifdef STATIC_BUILD
LDFLAGS += -static -static-libgcc -static-libstdc++
endif

# source files
# CIRCUITPY-CHANGE: extra files
SRC_C = \
	main.c \
	gccollect.c \
	shared/runtime/gchelper_generic.c \
	supervisor/stub/safe_mode.c \
	supervisor/stub/stack.c \
	supervisor/shared/translate/translate.c

# Add fmode when compiling with mingw gcc
COMPILER_TARGET := $(shell $(CC) -dumpmachine)
ifneq (,$(findstring mingw,$(COMPILER_TARGET)))
	SRC_C += windows-fmode.c
endif

OBJ = $(PY_CORE_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

# CIRCUITPY
$(BUILD)/supervisor/shared/translate/translate.o: $(HEADER_BUILD)/qstrdefs.generated.h $(HEADER_BUILD)/compressed_translations.generated.h
CIRCUITPY_MESSAGE_COMPRESSION_LEVEL = 1

include $(TOP)/py/mkrules.mk
