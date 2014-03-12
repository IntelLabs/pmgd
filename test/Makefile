CC := g++-4.8
RM := rm -f
ECHO := echo

LIB := ../lib/jarvis.lib
CLEANFILES := $(LIB)

INCLUDES := -I../include
OPT ?= -g

FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion
          -Wformat=2 -Wno-parentheses -Wno-conversion

CFLAGS := --std=c++11 $(INCLUDES) $(FFLAGS) $(WFLAGS) -DSTUBS $(OPT)

ifneq ($(findstring s,$(MAKEFLAGS)),)
    print = @$(ECHO) -e "\033[00;34m" [$(1)] $(2)"\033[00;00m"
endif

all: lib apitest alloctest soltest nodeedgetest listtest edgeindextest \
     propertytest propertypredicatetest filtertest txtest

apitest: apitest.o
	$(CC) -o $@ $<
CLEANFILES += apitest apitest.o

alloctest: alloctest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)
CLEANFILES += alloctest alloctest.o

soltest: soltest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += soltest soltest.o

nodeedgetest: nodeedgetest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += nodeedgetest nodeedgetest.o

listtest: listtest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIB)
CLEANFILES += listtest listtest.o

edgeindextest: edgeindextest.o
	$(call print,LINK,$@)
	$(CC) -o $@ $< ../lib/jarvis.lib
CLEANFILES += edgeindextest edgeindextest.o

propertytest: propertytest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += propertytest propertytest.o

propertypredicatetest: propertypredicatetest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += propertypredicatetest propertypredicatetest.o

filtertest: filtertest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += filtertest filtertest.o

txtest: txtest.o $(LIB)
	$(call print,LINK,$@)
	$(CC) -g -o $@ $< $(LIB)
CLEANFILES += txtest txtest.o

lib:
	make -C ../src OPT=$(OPT) install

clean:
	$(call print,CLEAN)
	$(RM) $(CLEANFILES)
	make -C ../src clean

%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<
