##
# Makefile for the Jarvis Lake tests.

# Tools.
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
    O_TEST :=
    O_LIB  := ../lib/
else
    O_ROOT := $O/
    O_TEST := $O/test/
    O_LIB  := $O/lib/
endif

# Targets.
TESTS := apitest alloctest soltest nodeedgetest listtest edgeindextest \
         propertytest propertypredicatetest propertychunktest filtertest \
         txtest propertylisttest \
         stringtabletest setproperty \
         load_tsv_test load_gson_test \
         avltest chunklisttest indextest indexstringtest

# Extra stuff to clean whose names cannot be derived from dependences.
CLEANFILES :=

# Translate targets into object and binary file names.
OBJS := $(TESTS:%=$(O_TEST)%.o)
BINS := $(TESTS:%=$(O_TEST)%)

# Additional places to look for include files.
INCLUDES := -I../include

# Default optimization level.
OPT ?= -O2

# Optimization and language options.
FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
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

# The default goal builds the libraries and utilities, creates the
# build directory, and builds all the targets.
all: lib util $(O_TEST) $(BINS)

$(O_TEST)apitest: $(O_TEST)apitest.o
	$(CC) -o $@ $<

$(O_TEST)alloctest: $(O_TEST)alloctest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)soltest: $(O_TEST)soltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)nodeedgetest: $(O_TEST)nodeedgetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)listtest: $(O_TEST)listtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)edgeindextest: $(O_TEST)edgeindextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)propertytest: $(O_TEST)propertytest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)propertypredicatetest: $(O_TEST)propertypredicatetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)propertychunktest: $(O_TEST)propertychunktest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)propertylisttest: $(O_TEST)propertylisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -static -o $@ $< $(LIBS)

$(O_TEST)filtertest: $(O_TEST)filtertest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)txtest: $(O_TEST)txtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)stringtabletest: $(O_TEST)stringtabletest.o $(LIBS)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)setproperty: $(O_TEST)setproperty.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)load_tsv_test: $(O_TEST)load_tsv_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)load_gson_test: $(O_TEST)load_gson_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -ljsoncpp

$(O_TEST)avltest: $(O_TEST)avltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)chunklisttest: $(O_TEST)chunklisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)indextest: $(O_TEST)indextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)indexstringtest: $(O_TEST)indexstringtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

# Build the Jarvis Lake library.
lib:
	make -C ../src O=$O OPT="$(OPT)" install

# Build the Jarvis Lake utilities.
util:
	make -C ../util O=$O OPT="$(OPT)" install

clean:
	$(call print,CLEAN)
	$(RM) $(LIBS) $(BINS) $(OBJS) $(DEPS) $(CLEANFILES)
	make -C ../src O=$O clean
	make -C ../util O=$O clean

# If the variable $(O_TEST) is non-empty and the corresponding
# directory does not exist then create it.
$(O_TEST):
	mkdir -p $@

# Rule for building an object file from a C++ file.
$(O_TEST)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Include dependency information if they are available.
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
