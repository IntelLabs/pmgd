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

LIBS := $(O_LIB)jarvis-util.lib $(O_LIB)jarvis.lib
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

OBJS := $(TOOLS:%=$(O_TOOLS)%.o)
BINS := $(TOOLS:%=$(O_TOOLS)%)

CLEANFILES += $(OBJS) $(BINS)

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

$(O_TOOLS):
	mkdir -p $@

$(O_TOOLS)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

CLEANFILES += $(DEPS)

clean:
	$(call print,CLEAN)
	$(RM) $(CLEANFILES)
