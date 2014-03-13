CC := g++-4.8
AR := ar
CP := cp -a
RM := rm -f
ECHO := echo

TARGET := jarvis.lib
INSTALLPATH := ../lib
CLEANFILES := 

SRC := graph.cc node.cc edge.cc property.cc stringid.cc \
       EdgeIndex.cc \
       PropertyList.cc StringTable.cc FixedAllocator.cc Allocator.cc \
       TransactionManager.cc transaction.cc linux.cc

# Translates source file names to object file names
OBJS := $(patsubst %.cc, %.o, $(SRC))

INCLUDES := -I../include
OPT ?= -O2
PM := $(shell if ! grep pcommit /proc/cpuinfo; then echo "-DNOPM"; fi)

FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion
          -Wformat=2 -Wno-parentheses -Wno-conversion

CFLAGS := --std=c++11 $(INCLUDES) $(OPT) $(FFLAGS) $(WFLAGS) $(PM) -MP -MMD

ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

install: $(TARGET)
	$(call print,INSTALL,$(INSTALLPATH))
	$(shell if ! test -d $(INSTALLPATH); then mkdir $(INSTALLPATH); fi)
	$(CP) $(TARGET) $(INSTALLPATH)

$(TARGET): $(OBJS)
	$(call print,AR,$@)
	$(AR) cr $@ $(OBJS)

clean:
	$(call print,CLEAN)
	$(RM) $(TARGET) $(OBJS) $(DEPS) $(CLEANFILES)

%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
