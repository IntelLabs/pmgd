##
# Makefile for the PMGD microbenchmarks.

# Eliminate all default rules.
.SUFFIXES:

# Do not delete intermediate files.
.SECONDARY: $(OBJS)

# Tools.
CC := g++
JC := javac
RM := rm -f
ECHO := echo

# Allow builds from a read-only source tree.
#
# $(O_ROOT) must precede all non-phony targets and dependences (without
# a separating slash).
ifeq ($O,)
    O_ROOT :=
else
    O_ROOT := $O/
endif

# Targets.
MICROS := maxdegree maxinoutdegree diameter pdiameter shortestpath triangles \
        findnodes emailtargets phrase1orphrase2 findemailwithattachment
JAVA_MICROS := Phrase1OrPhrase2 FindNodes EmailTargets MaxInOutDegree \
		FindEmailWithAttachment ShortestPath Triangles

# Extra stuff to clean whose names cannot be derived from dependences.
CLEANFILES :=

# Translate targets into object file names.
OBJS := $(MICROS:%=$(O_ROOT)%.o)
BINS := $(MICROS:%=$(O_ROOT)%) $(JAVA_MICROS:%=$(O_ROOT)%.class)
CLEANFILES += $(O_ROOT)Triangles\$$Degree.class

# Where to find PMGD
JLINCROOT := /opt/intel/vdms/pmgd
JLLIBROOT := /opt/intel/vdms/pmgd

# Additional places to look for include files.
INCLUDES := -I"$(JLINCROOT)/include" -I"$(JLINCROOT)/util"

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
          -Wno-parentheses -Wno-conversion

# Flags for C++ compilation.
CFLAGS := --std=c++11 $(INCLUDES) $(OPT) $(FFLAGS) $(WFLAGS) -MP -MMD

# PMGD library.
LIBS := "$(JLLIBROOT)/lib/pmgd.lib" "$(JLLIBROOT)/lib/pmgd-util.lib"

# Sprinkle noise in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) [$(1)] $(2)
endif

# Default goal: build everything.
all: $(BINS)

clean:
	$(call print,CLEAN)
	$(RM) $(BINS) $(OBJS) $(DEPS) $(CLEANFILES)

# Rule for building an object file from a C++ file.
$(O_ROOT)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(O_ROOT)pdiameter.o: pdiameter.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $< -pthread

# Rule for building a binary from a single object file.
%: %.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

# Rule for building a Java application
%.class: %.java $(JLLIBROOT)/lib/pmgd-jni.jar
	$(call print,JC,$@)
	$(JC) -cp "$(JLLIBROOT)/lib/pmgd-jni.jar" $<

pdiameter: pdiameter.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -pthread

# Include dependency information if they are available.
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
