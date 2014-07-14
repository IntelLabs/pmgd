CC := g++-4.8
RM := rm -f
ECHO := echo

ifeq ($O,)
    O_ROOT  := ../
    O_TOOLS :=
    O_LIB   := ../lib/
else
    O_ROOT  := $O/
    O_TOOLS := $O/tools/
    O_LIB   := $O/lib/
endif

TOOLS := mkgraph loadgraph dumpgraph
CLEANFILES :=

OBJS := $(TOOLS:%=$(O_TOOLS)%.o)
BINS := $(TOOLS:%=$(O_TOOLS)%)

INCLUDES := -I../include -I../util
OPT ?= -O2

FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings \
          -Wformat=2 -Wno-parentheses -Wno-conversion

CFLAGS := --std=c++11 $(INCLUDES) $(FFLAGS) $(WFLAGS) $(OPT) \
          $(PM) -MP -MMD

LIBS := $(O_LIB)jarvis-util.lib $(O_LIB)jarvis.lib

ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

all: $(O_TOOLS) $(BINS)

$(O_TOOLS)mkgraph: $(O_TOOLS)mkgraph.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TOOLS)loadgraph: $(O_TOOLS)loadgraph.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -ljsoncpp

$(O_TOOLS)dumpgraph: $(O_TOOLS)dumpgraph.o
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

clean:
	$(call print,CLEAN)
	$(RM) $(BINS) $(OBJS) $(DEPS) $(CLEANFILES)

$(O_TOOLS):
	mkdir -p $@

$(O_TOOLS)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
