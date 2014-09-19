##
# Makefile for the Jarvis Lake utilities.

# Tools.
CC := g++-4.8
AR := ar
CP := cp -a
RM := rm -f
ECHO := echo

# Allow builds from a read-only source tree.
#
# $(O_UTIL) must precede all non-phony targets and dependences (without
# a separating slash).  Use $(O_ROOT) when the target is a file or
# directory off the root directory of the project.  O_ROOT and O_UTIL
# must either be empty or end with a slash.
ifeq ($O,)
    O_ROOT := ../
    O_UTIL :=
else
    O_ROOT := $O/
    O_UTIL := $O/util/
endif

# Targets.
SLIBRARY := $(O_UTIL)jarvis-util.lib
DLIBRARY := $(O_UTIL)libjarvis-util.so

# Where the targets should be installed.
INSTALLPATH := $(O_ROOT)lib

# Extra stuff to clean whose names cannot be derived from dependences.
CLEANFILES :=

# List of source files.
SRCS := exception.cc text.cc dump.cc load_tsv.cc load_gson.cc loader.y scanner.l

# Translate source file names to object file names.
OBJS := $(patsubst %.cc,$(O_UTIL)%.o, \
          $(patsubst %.y,$(O_UTIL)%.o, \
            $(patsubst %.l,$(O_UTIL)%.o, $(SRCS))))

# Add the output of flex and bison to the list of files to clean.
CLEANFILES += $(patsubst %.y,$(O_UTIL)%.cc, \
                $(patsubst %.l,$(O_UTIL)%.cc, \
                  $(filter %.y %.l, $(SRCS))))
CLEANFILES += $(patsubst %.y,$(O_UTIL)%.hh, \
                $(patsubst %.l,$(O_UTIL)%.hh, \
                  $(filter %.y %.l, $(SRCS))))

# Additional places to look for include files.
INCLUDES := -I../include

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
CFLAGS := --std=c++11 $(INCLUDES) $(OPT) $(FFLAGS) $(WFLAGS) -MP -MMD -fPIC

# Add a little bit of colorful noise in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

# The default goal creates the build directory, builds the targets,
# creates the install directory, and copies the targets there.
install: $(O_UTIL) $(SLIBRARY) $(DLIBRARY)
	$(shell if ! test -d $(INSTALLPATH); then mkdir $(INSTALLPATH); fi)
	$(call print,INSTALL,$(SLIBRARY))
	$(CP) $(SLIBRARY) $(INSTALLPATH)
	$(call print,INSTALL,$(DLIBRARY))
	$(CP) $(DLIBRARY) $(INSTALLPATH)

$(SLIBRARY): $(OBJS)
	$(call print,AR,$@)
	$(AR) cr $@ $(OBJS)

$(DLIBRARY): $(OBJS)
	$(call print,AR,$@)
	$(CC) -shared -o $(DLIBRARY) $(OBJS)

clean:
	$(call print,CLEAN)
	$(RM) $(SLIBRARY) $(DLIBRARY) $(OBJS) $(DEPS) $(CLEANFILES)
	$(RM) $(INSTALLPATH)/$(SLIBRARY) $(INSTALLPATH)/$(DLIBRARY)

# If the variable $(O_UTIL) is non-empty and the corresponding
# directory does not exist then create it.
$(O_UTIL):
	mkdir -p $@

# Rule for building an object file from a C++ file.
$(O_UTIL)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Special rule for building an object file from a generated C++ file.
$(O_UTIL)%.o: $(O_UTIL)%.cc
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Rule for generating a C++ file from a lex file.
$(O_UTIL)scanner.cc: scanner.l $(MAKEFILE_LIST)
	$(call print,LEX,$<)
	flex $(LFLAGS) -o$@ --header-file=$*.hh $<

# Rule for generating a C++ file from a yacc file.
$(O_UTIL)loader.cc: loader.y $(MAKEFILE_LIST)
	$(call print,YACC,$<)
	bison $(YFLAGS) -d -o $@ $<

# Include dependency information if they are available.
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
