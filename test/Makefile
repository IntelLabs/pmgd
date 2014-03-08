CC := g++-4.8
RM := rm -f

LIB := ../lib/jarvis.lib
CLEANFILES :=

INCLUDES := -I../include -I../src
OPT ?= -g

FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion
          -Wformat=2 -Wno-parentheses -Wno-conversion

CFLAGS := --std=c++11 $(INCLUDES) $(FFLAGS) $(WFLAGS) -DSTUBS $(OPT)

all: lib apitest alloctest soltest nodeedgetest listtest edgeindextest \
     propertytest propertypredicatetest filtertest txtest

apitest: apitest.o
	$(CC) -o $@ $<
CLEANFILES += apitest apitest.o

alloctest: alloctest.o $(LIB)
	$(CC) -o $@ $< $(LIB)
CLEANFILES += alloctest alloctest.o

soltest: soltest.o $(LIB)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += soltest soltest.o

nodeedgetest: nodeedgetest.o $(LIB)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += nodeedgetest nodeedgetest.o

listtest: listtest.o $(LIB)
	$(CC) -o $@ $< $(LIB)
CLEANFILES += listtest listtest.o

edgeindextest: edgeindextest.o
	$(CC) -o $@ $< ../lib/jarvis.lib
CLEANFILES += edgeindextest edgeindextest.o

propertytest: propertytest.o $(LIB)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += propertytest propertytest.o

propertypredicatetest: propertypredicatetest.o $(LIB)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += propertypredicatetest propertypredicatetest.o

filtertest: filtertest.o $(LIB)
	$(CC) $(OPT) -o $@ $< $(LIB)
CLEANFILES += filtertest filtertest.o

txtest: txtest.o $(LIB)
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
