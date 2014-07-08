##
# Makefile for the Jarvis Lake library.

# Tools.
CC := g++-4.8
AR := ar
CP := cp -a
RM := rm -f
ECHO := echo

# Allow builds from a read-only source tree.
#
# $(O_SRC) must precede all non-phony targets and dependences (without
# a separating slash).  Use $(O_ROOT) when the target is a file or
# directory off the root directory of the project.  O_ROOT and O_SRC
# must either be empty or end with a slash.
ifeq ($O,)
    O_ROOT := ../
    O_SRC  :=
else
    O_ROOT := $O/
    O_SRC  := $O/src/
endif

# Target.
TARGET := $(O_SRC)jarvis.lib

# Where the target should be installed.
INSTALLPATH := $(O_ROOT)lib

# Extra stuff to clean whose names cannot be derived from dependences.
CLEANFILES := 

# List of source files.
SRCS := graph.cc node.cc edge.cc property.cc stringid.cc \
        EdgeIndex.cc \
        PropertyList.cc StringTable.cc FixedAllocator.cc Allocator.cc \
        TransactionManager.cc transaction.cc \
        IndexString.cc AvlTreeIndex.cc IndexManager.cc Index.cc \
        linux.cc

# Translates source file names to object file names.
OBJS := $(patsubst %.cc, $(O_SRC)%.o, $(SRCS))

# Additional places to look for include files.
INCLUDES := -I../include

# Default optimization level.
OPT ?= -O2

# Are we building on a platform with support for persistent memory?
PM ?= $(shell if ! grep pcommit /proc/cpuinfo; then echo "-DNOPM"; fi)

# Optimization and language options.
FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

# Warning options.
WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings \
          -Wno-parentheses -Wno-conversion

# Flags for C++ compilation.
CFLAGS := --std=c++11 $(INCLUDES) $(OPT) $(FFLAGS) $(WFLAGS) $(PM) -MP -MMD

# Add a little bit of colorful noise in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

# The default goal creates the build directory, builds the target,
# creates the install directory, and copies the target there.
install: $(O_SRC) $(TARGET)
	$(call print,INSTALL,$(INSTALLPATH))
	$(shell if ! test -d $(INSTALLPATH); then mkdir $(INSTALLPATH); fi)
	$(CP) $(TARGET) $(INSTALLPATH)

$(TARGET): $(OBJS)
	$(call print,AR,$@)
	$(AR) cr $@ $(OBJS)

clean:
	$(call print,CLEAN)
	$(RM) $(TARGET) $(OBJS) $(DEPS) $(CLEANFILES)

# If the variable $(O_SRC) is non-empty and the corresponding
# directory does not exist then create it.
$(O_SRC):
	mkdir -p $@

# Rule for building an object file from a C++ file.
$(O_SRC)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Include dependency information if they are available.
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
