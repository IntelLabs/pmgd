##
# Makefile for the Jarvis Lake Java bindings

#Tools.
CC := g++-4.8
RM := rm -f
ECHO := echo

# Allow builds from a read-only source tree.
#
# $(O_TEST) must precede all non-phony targets and dependences (without
# a separating slash).  Use $(O_ROOT) when the target is a file or
# directory off the root directory of the project.  $(O_LIB) specifies
# the location of the Jarvis Lake library against which we build some
# of our tests.  O_ROOT, O_TEST, and O_LIB must either be empty or
# end with a slash.
ifeq ($O,)
    O_ROOT := ../
    O_LIB  := ../lib/
else
    O_ROOT := $O/
    O_LIB  := $O/lib/
endif

# Targets.
CLASSES :=  Property Node NodeIterator Edge Transaction Graph
JAVAFILES := $(CLASSES:%=%.java)
CCFILES := $(CLASSES:%=%.cc)
OBJS := $(CLASSES:%=%.o)

# Extra stuff to clean whose names cannot be derived from dependences.
CLEANFILES := libjarvis-jni.so *~

# Additional places to look for include files.
INCLUDES := -I$(O_ROOT)include  -I$(O_ROOT)/util 
JAVA_INCLUDES := -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

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
CFLAGS := --std=c++11 $(INCLUDES) $(JAVA_INCLUDES) $(FFLAGS) $(WFLAGS) $(OPT) \
          $(PM) -MP -MMD

# Jarvis Lake libraries.
LIBS := -ljarvis-util -ljarvis -ljsoncpp
LDFLAGS += -L$(O_LIB) -L/usr/lib

# Add a little bit of colorful noise in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

# For binding with a java library
LIBRARY := libjarvis-jni.so

# The default goal builds the libraries and utilities, creates the
# build directory, and builds all the targets.

all: $(O_LIB)libjarvis.so $(O_LIB)libjarvis-util.so $(LIBRARY) test

$(O_LIB)libjarvis.so: lib

$(O_LIB)libjarvis-util.so: $(O_LIB)jarvis.lib util

headers: $(JAVAFILES)
	javac $(JAVAFILES)
	javah -jni $(CLASSES)

$(LIBRARY): $(CCFILES)
	$(CC) $(CFLAGS)  -fPIC -c $(CCFILES) $(LDFLAGS) $(LIBS)
	$(CC) $(CFLAGS)  -shared -o $(LIBRARY) $(OBJS) $(LDFLAGS) $(LIBS)

test: $(LIBRARY)
	javac $(JAVAFILES) BindingsTest.java


# Build the Jarvis Lake library.
lib:
	make -C $(O_ROOT)src O=$O OPT="$(OPT)" install

# Build the Jarvis Lake utilities.
util:
	make -C $(O_ROOT)util O=$O OPT="$(OPT)" install

clean:
	$(call print,CLEAN)
	$(RM) $(BINS) $(OBJS) $(DEPS) $(CLEANFILES)
	make -C $(O_ROOT)src O=$O clean
	make -C $(O_ROOT)util O=$O clean

# If the variable $(O_BINDINGS) is non-empty and the corresponding
# directory does not exist then create it.
$(O_BINDINGS):
	mkdir -p $@

# Rule for building an object file from a C++ file.
$(O_BINDINGS)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Include dependency information if they are available.
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
