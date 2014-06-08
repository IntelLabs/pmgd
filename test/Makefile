CC := g++-4.8
RM := rm -f
ECHO := echo

LIBS := ../lib/jarvis-util.lib ../lib/jarvis.lib
CLEANFILES := $(LIBS)

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

alloctest: alloctest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

soltest: soltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

nodeedgetest: nodeedgetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

listtest: listtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

edgeindextest: edgeindextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

propertytest: propertytest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

propertypredicatetest: propertypredicatetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

propertychunktest: propertychunktest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

propertylisttest: propertylisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -static -o $@ $< $(LIBS)

filtertest: filtertest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

txtest: txtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

stringtabletest: stringtabletest.o $(LIBS)
	$(CC) $(OPT) -o $@ $< $(LIBS)

setproperty: setproperty.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

load_tsv_test: load_tsv_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

load_gson_test: load_gson_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -ljsoncpp

avltest: avltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

chunklisttest: chunklisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

indextest: indextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

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

