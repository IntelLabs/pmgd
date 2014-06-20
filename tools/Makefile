CC := g++-4.8
RM := rm -f
ECHO := echo

O := ..

LIBS := $O/lib/jarvis-util.lib $O/lib/jarvis.lib
CLEANFILES :=

INCLUDES := -I../include -I../util
OPT ?= -O2

FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings \
          -Wformat=2 -Wno-parentheses -Wno-conversion

CFLAGS := --std=c++11 $(INCLUDES) $(FFLAGS) $(WFLAGS) $(OPT) \
          $(PM) -MP -MMD

ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

TOOLS := mkgraph loadgraph dumpgraph

OBJS := $(TOOLS:%=$O/tools/%.o)
BINS := $(TOOLS:%=$O/tools/%)

CLEANFILES += $(OBJS) $(BINS)

all: $O/tools $(BINS)

$O/tools/mkgraph: $O/tools/mkgraph.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/tools/loadgraph: $O/tools/loadgraph.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -ljsoncpp

$O/tools/dumpgraph: $O/tools/dumpgraph.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/tools/%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

$O/tools:
	mkdir -p $@

$O/tools/%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

CLEANFILES += $(DEPS)

clean:
	$(call print,CLEAN)
	$(RM) $(CLEANFILES)
