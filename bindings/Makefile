##
# Makefile for the Jarvis Lake Java bindings

# Eliminate all default rules.
.SUFFIXES:

# Tools.
CC := g++-4.8
CP := cp -a
RM := rm -f
ECHO := echo
JC := javac
JH := javah
JAR := jar

# Allow builds from a read-only source tree.
#
# $(O_BINDINGS) must precede all non-phony targets and dependences (without
# a separating slash).  Use $(O_ROOT) when the target is a file or
# directory off the root directory of the project.  $(O_LIB) specifies
# the location of the Jarvis Lake library against which we build some
# of our tests.  O_ROOT, O_TEST, and O_LIB must either be empty or
# end with a slash.
ifeq ($O,)
    O_ROOT     := ../
    O_BINDINGS :=
    O_LIB      := ../lib/
else
    O_ROOT     := $O/
    O_BINDINGS := $O/bindings/
    O_LIB      := $O/lib/
endif

# Targets.
DLIBRARY := $(O_BINDINGS)libjarvis-jni.so
JLIBRARY := $(O_BINDINGS)jarvis-jni.jar

# Where the targets should be installed.
INSTALLPATH := $(O_ROOT)lib

# Extra stuff to clean whose names cannot be derived from dependences.
CLEANFILES :=

# List of classes implementing the bindings.
CLASSES := Property PropertyIterator Node NodeIterator Edge EdgeIterator \
           Transaction Graph Exception PropertyPredicate

# Translate source file names.
JSRCS := $(CLASSES:%=$(O_BINDINGS)%.java)
JOBJS := $(CLASSES:%=$(O_BINDINGS)%.class)
CSRCS := $(CLASSES:%=$(O_BINDINGS)%.cc)
COBJS := $(CLASSES:%=$(O_BINDINGS)%.o)

JOBJS += $(O_BINDINGS)Graph\$$OpenOptions.class
JOBJS += $(O_BINDINGS)Graph\$$IndexOptions.class
JOBJS += $(O_BINDINGS)Node\$$Direction.class
JOBJS += $(O_BINDINGS)PropertyPredicate\$$op_t.class

# Additional places to look for include files.
INCLUDES := -I$(O_BINDINGS). -I../include -I../util
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
          $(PM) -MP -MMD -fPIC

# Jarvis Lake libraries.
CLIBS := -L$(O_LIB) -ljarvis-util -ljarvis

# Add a little bit of colorful noise in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

# The default goal creates the build directory, builds the targets,
# creates the install directory, and copies the targets there.
install: $(O_BINDINGS) $(DLIBRARY) $(JLIBRARY)
	$(shell if ! test -d $(INSTALLPATH); then mkdir $(INSTALLPATH); fi)
	$(call print,INSTALL,$(DLIBRARY))
	$(CP) $(DLIBRARY) $(INSTALLPATH)
	$(call print,INSTALL,$(JLIBRARY))
	$(CP) $(JLIBRARY) $(INSTALLPATH)

$(DLIBRARY): $(COBJS)
	$(CC) -shared -o $@ $(COBJS) $(CLIBS)

$(JLIBRARY): $(JOBJS)
	cd $(O_BINDINGS).; $(JAR) cf $@ $(subst $(O_BINDINGS),,$(JOBJS))

clean:
	$(call print,CLEAN)
	$(RM) $(DLIBRARY) $(JLIBRARY) $(COBJS) $(JOBJS) $(DEPS) $(CLEANFILES)
	$(RM) $(INSTALLPATH)/$(DLIBRARY) $(INSTALLPATH)/$(JLIBRARY)

# If the variable $(O_BINDINGS) is non-empty and the corresponding
# directory does not exist then create it.
$(O_BINDINGS):
	mkdir -p $@

$(O_BINDINGS)Graph\$$OpenOptions.class: Graph.java
	$(call print,COMPILE,$@)
	$(JC) -d $(O_BINDINGS). $<

$(O_BINDINGS)Graph\$$IndexOptions.class: Graph.java
	$(call print,COMPILE,$@)
	$(JC) -d $(O_BINDINGS). $<

$(O_BINDINGS)Node\$$Direction.class: Node.java
	$(call print,COMPILE,$@)
	$(JC) -d $(O_BINDINGS). $<

$(O_BINDINGS)PropertyPredicate\$$op_t.class: PropertyPredicate.java
	$(call print,COMPILE,$@)
	$(JC) -d $(O_BINDINGS). $<

# Rule for building an object file from a C++ file.
$(O_BINDINGS)%.o: %.cc $(O_BINDINGS)%.h $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Rule for building a JNI header file for a class.
$(O_BINDINGS)%.h: %.java $(MAKEFILE_LIST)
	$(call print,JAVAH,$@)
	$(JH) -jni -o $@ $*

# Rule for building bytecodes from a Java file.
$(O_BINDINGS)%.class: %.java $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(JC) -d $(O_BINDINGS). $<

# Include dependency information if they are available.
DEPS := $(COBJS:%.o=%.d)
-include $(DEPS)
