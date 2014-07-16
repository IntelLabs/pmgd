##
# Makefile for the Jarvis Lake tools.

# Tools.
CC := g++-4.8
RM := rm -f
ECHO := echo

# Allow builds from a read-only source tree.
#
# $(O_TOOLS) must precede all non-phony targets and dependences (without
# a separating slash).  Use $(O_ROOT) when the target is a file or
# directory off the root directory of the project.  $(O_LIB) specifies
# the location of the Jarvis Lake library against which we build our
# tools.  O_ROOT, O_TOOLS, and O_LIB must either be empty or end with
# a slash.
ifeq ($O,)
    O_ROOT  := ../
    O_TOOLS :=
    O_LIB   := ../lib/
else
    O_ROOT  := $O/
    O_TOOLS := $O/tools/
    O_LIB   := $O/lib/
endif

# Targets.
TOOLS := mkgraph loadgraph dumpgraph

# Extra stuff to clean whose names cannot be derived from dependences.
CLEANFILES :=

# Translate targets into object and binary file names.
OBJS := $(TOOLS:%=$(O_TOOLS)%.o)
BINS := $(TOOLS:%=$(O_TOOLS)%)

# Additional places to look for include files.
INCLUDES := -I../include -I../util

# Default optimization level.
OPT ?= -O3

# Omit the frame pointer unless we are profiling.
ifeq ($(findstring -pg,$(OPT)),)
OMIT_FRAME_POINTER := -fomit-frame-pointer
endif

# Optimization and language options.
FFLAGS := $(OMIT_FRAME_POINTER) -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

# Warning options.
WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings \
          -Wformat=2 -Wno-parentheses -Wno-conversion

# Flags for C++ compilation.
CFLAGS := --std=c++11 $(INCLUDES) $(FFLAGS) $(WFLAGS) $(OPT) \
          $(PM) -MP -MMD

# Jarvis Lake libraries.
LIBS := $(O_LIB)jarvis-util.lib $(O_LIB)jarvis.lib

# Add a little bit of colorful noise in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

# The default goal creates the build directory and builds all the targets.
all: $(O_TOOLS) $(BINS)

$(O_LIB)jarvis.lib: lib

$(O_LIB)jarvis-util.lib:$(O_LIB)jarvis.lib util

$(O_TOOLS)mkgraph: $(O_TOOLS)mkgraph.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TOOLS)loadgraph: $(O_TOOLS)loadgraph.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -ljsoncpp

$(O_TOOLS)dumpgraph: $(O_TOOLS)dumpgraph.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

# Build the Jarvis Lake library.
lib:
	make -C ../src O=$O OPT="$(OPT)" install

# Build the Jarvis Lake utilities.
util:
	make -C ../util O=$O OPT="$(OPT)" install

clean:
	$(call print,CLEAN)
	$(RM) $(BINS) $(OBJS) $(DEPS) $(CLEANFILES)
	make -C ../src O=$O clean
	make -C ../util O=$O clean

# If the variable $(O_TOOLS) is non-empty and the corresponding
# directory does not exist then create it.
$(O_TOOLS):
	mkdir -p $@

# Rule for building an object file from a C++ file.
$(O_TOOLS)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Include dependency information if they are available.
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
