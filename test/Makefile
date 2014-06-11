CC := g++-4.8
RM := rm -f
ECHO := echo

LIB := ../lib/jarvis-util.lib ../lib/jarvis.lib
CLEANFILES := $(LIB)

INCLUDES := -I../include
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

TESTS := apitest alloctest soltest nodeedgetest listtest edgeindextest \
         propertytest propertypredicatetest propertychunktest filtertest \
         txtest propertylisttest \
         stringtabletest setproperty \
         load_tsv_test load_gson_test \
         avltest chunklisttest indextest

OBJS := $(TESTS:%=%.o)

CLEANFILES += $(TESTS) $(OBJS)

all: lib util $(TESTS)

apitest: apitest.o
	$(CC) -o $@ $<

alloctest: alloctest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)

soltest: soltest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

nodeedgetest: nodeedgetest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

listtest: listtest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)

edgeindextest: edgeindextest.o
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)

propertytest: propertytest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

propertypredicatetest: propertypredicatetest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

propertychunktest: propertychunktest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

propertylisttest: propertylisttest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -static -o $@ $< $(LIB)

filtertest: filtertest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

txtest: txtest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

stringtabletest: stringtabletest.o $(LIB)
	$(CC) $(OPT) -o $@ $< $(LIB)

setproperty: setproperty.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

load_tsv_test: load_tsv_test.o $(LIB) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)

load_gson_test: load_gson_test.o $(LIB) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB) -ljsoncpp

avltest: avltest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)

chunklisttest: chunklisttest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)

indextest: indextest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)

lib:
	make -C ../src OPT=$(OPT) install

util:
	make -C ../util OPT=$(OPT) install

%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

CLEANFILES += $(DEPS)

clean:
	$(call print,CLEAN)
	$(RM) $(CLEANFILES)
	make -C ../src clean
	make -C ../util clean

