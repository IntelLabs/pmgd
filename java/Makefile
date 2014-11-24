##
# Makefile for the Jarvis Lake Java bindings

# Eliminate all default rules.
.SUFFIXES:

# Ask make not to delete these intermediate files
.SECONDARY: $(CHDRS)

#MAKEFILE := $(lastword $(MAKEFILE_LIST))

# Tools.
CC := g++-4.8
CP := cp -a
RM := rm -f
ECHO := echo
MKDIR := mkdir
TOUCH := touch
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
JOBJS := $(CLASSES:%=$(O_BINDINGS)jarvis/%.class)
CSRCS := $(CLASSES:%=$(O_BINDINGS)%.cc)
COBJS := $(CLASSES:%=$(O_BINDINGS)%.o)
CHDRS := $(CLASSES:%=%(O_BINDINGS)%.h)

JOBJS += $(O_BINDINGS)jarvis/Graph\$$OpenOptions.class
JOBJS += $(O_BINDINGS)jarvis/Graph\$$IndexOptions.class
JOBJS += $(O_BINDINGS)jarvis/Node\$$Direction.class
JOBJS += $(O_BINDINGS)jarvis/PropertyPredicate\$$op_t.class

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
          -Wformat=2 -Wno-parentheses -Wno-conversion \
          -Wmissing-declarations

# Flags for C++ compilation.
CFLAGS := --std=c++11 $(INCLUDES) $(JAVA_INCLUDES) $(FFLAGS) $(WFLAGS) $(OPT) \
          $(PM) -MP -MMD -fPIC

# Add a little bit of colorful noise in silent mode.
ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

# The default goal creates the build directory, builds the targets,
# creates the install directory, and copies the targets there.
install: $(O_BINDINGS) $(INSTALLPATH) \
         $(INSTALLPATH)/$(JLIBRARY) $(INSTALLPATH)/$(DLIBRARY)

$(INSTALLPATH)/$(JLIBRARY): $(JLIBRARY)
	$(call print,INSTALL,$(DLIBRARY))
	$(CP) $(JLIBRARY) $(INSTALLPATH)

$(INSTALLPATH)/$(DLIBRARY): $(DLIBRARY)
	$(call print,INSTALL,$(JLIBRARY))
	$(CP) $(DLIBRARY) $(INSTALLPATH)

$(DLIBRARY): $(COBJS) $(O_LIB)jarvis-util.lib $(O_LIB)jarvis.lib
	$(CC) -shared -o $@ $(COBJS) $(O_LIB)jarvis-util.lib $(O_LIB)jarvis.lib

$(JLIBRARY): $(JOBJS)
	cd $(O_BINDINGS).; $(JAR) cf $@ $(subst $(O_BINDINGS),,$(JOBJS))

$(O_LIB)jarvis.lib:
	make -C ../src

$(O_LIB)jarvis-util.lib:
	make -C ../util

clean:
	$(call print,CLEAN)
	$(RM) $(DLIBRARY) $(JLIBRARY) $(COBJS) $(JOBJS) $(DEPS) $(CLEANFILES)
	$(RM) $(INSTALLPATH)/$(DLIBRARY) $(INSTALLPATH)/$(JLIBRARY)

# If the variable $(O_BINDINGS) is non-empty and the corresponding
# directory does not exist then create it.
$(O_BINDINGS):
	$(MKDIR) -p $@

$(INSTALLPATH):
	$(MKDIR) -p $(INSTALLPATH)

# Rule for building an object file from a C++ file.
$(O_BINDINGS)%.o: %.cc $(O_BINDINGS)%.h $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Rule for building a JNI header file for a class.
$(O_BINDINGS)%.h: $(O_BINDINGS)jarvis/%.class $(MAKEFILE)
	$(call print,JAVAH,$@)
	$(JH) -jni -cp $(O_BINDINGS). -o $@ jarvis.$*
	$(TOUCH) -m -r $< $@

# Rule for building bytecodes from a Java file.
$(O_BINDINGS)jarvis/%.class: %.java $(MAKEFILE)
	$(call print,COMPILE,$@)
	$(JC) -cp $(O_BINDINGS). -d $(O_BINDINGS). $<

# Include dependency information if they are available.
DEPS := $(COBJS:%.o=%.d)
-include $(DEPS)

# Explicit list of dependencies among the java source files.
$(O_BINDINGS)jarvis/Graph.class: \
    $(O_BINDINGS)jarvis/Exception.class \
    $(O_BINDINGS)jarvis/Node.class \
    $(O_BINDINGS)jarvis/Edge.class \
    $(O_BINDINGS)jarvis/NodeIterator.class \
    $(O_BINDINGS)jarvis/EdgeIterator.class \
    $(O_BINDINGS)jarvis/PropertyPredicate.class

$(O_BINDINGS)jarvis/NodeIterator.class: \
    $(O_BINDINGS)jarvis/Exception.class \
    $(O_BINDINGS)jarvis/Node.class \
    $(O_BINDINGS)jarvis/Property.class \
    $(O_BINDINGS)jarvis/PropertyIterator.class

$(O_BINDINGS)jarvis/Property.class: \
    $(O_BINDINGS)jarvis/Exception.class

$(O_BINDINGS)jarvis/PropertyIterator.class: \
    $(O_BINDINGS)jarvis/Exception.class \
    $(O_BINDINGS)jarvis/Property.class

$(O_BINDINGS)jarvis/Transaction.class: \
    $(O_BINDINGS)jarvis/Exception.class \
    $(O_BINDINGS)jarvis/Graph.class

# Build these three sources together because of mutual dependencies.
.SECONDARY: Node.build
$(O_BINDINGS)jarvis/Node.class: Node.build
$(O_BINDINGS)jarvis/Node\$$Direction.class: Node.build
$(O_BINDINGS)jarvis/Edge.class: Node.build
$(O_BINDINGS)jarvis/EdgeIterator.class: Node.build

Node.build: \
    Node.java \
    Edge.java \
    EdgeIterator.java \
    $(O_BINDINGS)jarvis/Exception.class \
    $(O_BINDINGS)jarvis/Property.class \
    $(O_BINDINGS)jarvis/PropertyIterator.class \
    $(MAKEFILE)
	$(call print,COMPILE,$@)
	$(JC) -cp $(O_BINDINGS). -d $(O_BINDINGS). Node.java Edge.java EdgeIterator.java

# Rules for compiling a single Java source file into multiple classes.
.SECONDARY: Graph.build
$(O_BINDINGS)jarvis/Graph.class: Graph.build
$(O_BINDINGS)jarvis/Graph\$$OpenOptions.class: Graph.build
$(O_BINDINGS)jarvis/Graph\$$IndexOptions.class: Graph.build

Graph.build: Graph.java $(MAKEFILE)
	$(call print,COMPILE,$@)
	$(JC) -cp $(O_BINDINGS). -d $(O_BINDINGS). $<

.SECONDARY: PropertyPredicate.build
$(O_BINDINGS)jarvis/PropertyPredicate.class: PropertyPredicate.build
$(O_BINDINGS)jarvis/PropertyPredicate\$$op_t.class: PropertyPredicate.build

PropertyPredicate.build: PropertyPredicate.java $(MAKEFILE)
	$(call print,COMPILE,$@)
	$(JC) -cp $(O_BINDINGS). -d $(O_BINDINGS). $<
