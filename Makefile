##
# Main Makefile for the Jarvis Lake library.

##
# Tunables.

# Default optimization level.
OPT ?= -O3

# Are we building on a platform with support for persistent memory?
PM ?= $(shell if ! grep pcommit /proc/cpuinfo; then echo "-DNOPM"; fi)

# By default let us treat warnings as an error.
WERROR ?= -Werror

##
# The rest.

# Delete all default suffixes.
.SUFFIXES:

# Do not delete any intermediate files.
.SECONDARY:

# Tools.
CC := g++-4.8
LEX := flex
YACC := bison
AR := ar
RM := rm -f
RMDIR := rmdir
TRUE := true
ECHO := echo
INSTALL := install

# Be a little noisy in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) "[$(1)] $(2)"
endif

##
# Defaults for all builds.
#
# Define overrides in the subordinate Makefiles.

# Additional places to look for include files.
INCLUDES := -Iinclude

# Omit the frame pointer unless we are profiling.
ifeq ($(findstring -pg,$(OPT)),)
OMIT_FRAME_POINTER := -fomit-frame-pointer
endif

# Optimization and language options.
FFLAGS := $(OMIT_FRAME_POINTER) -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics \
          -fnon-call-exceptions

# Warning options.
WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings \
          -Wno-parentheses -Wno-conversion $(WERROR)

# Flags for C++ compilation.
CFLAGS := --std=c++11 $(INCLUDES) $(OPT) $(FFLAGS) $(WFLAGS) $(PM) -MP -MMD

# Define CLEANFILES and CLEANDIRS as immediate variables!
CLEANFILES :=
CLEANDIRS :=

##
# "all" is our default target.
#
# "all" redirects to "doall" which must be defined after the includes.
PHONY += all
all: doall

# Include subordinate Makefiles.
include src/Makeconf
include util/Makeconf
include tools/Makeconf
include test/Makeconf

# The default rule builds all libraries and the utility binaries.
PHONY += doall
doall: $(LIBS) $(BINS) $(TESTS)

# Clean everything.
PHONY += clean
clean:
	$(call print,CLEAN)
	$(RM) $(LIBS) $(BINS) $(TESTS) $(OBJS) $(DEPS) $(CLEANFILES)
	for i in "$(CLEANDIRS)"; do test -e $$i && $(RMDIR) $$i || $(TRUE); done

# Default rule for building an object file from a C++ file.
%.o: %.cc $(MAKEFILE_LIST)
	$(call print,CC,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Default rule for generating a C++ file from a lex file.
%.cc: %.l $(MAKEFILE_LIST)
	$(call print,LEX,$@)
	$(LEX) $(LFLAGS) -o$@ $<

# Default rule for generating a C++ file from a yacc file.
%.cc: %.y $(MAKEFILE_LIST)
	$(call print,YACC,$@)
	$(YACC) $(YFLAGS) -d -o $@ $<

# Include dependency information if they are available.
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

# All names in the variable $(PHONY) are so.
.PHONY: $(PHONY)

# Don't attempt to rebuild Makefile.
Makefile : ;
