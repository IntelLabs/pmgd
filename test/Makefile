CC := g++-4.8
RM := rm -f

LIB := ../lib/jarvis.lib
CLEANFILES := apitest apitest.o soltest soltest.o

INCLUDES := -I../include
OPT ?= -O2

FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion
          -Wformat=2 -Wno-parentheses -Wno-conversion

CFLAGS := --std=c++11 $(INCLUDES) $(OPT) $(FFLAGS) $(WFLAGS) -DSTUBS

all: lib apitest alloctest soltest

apitest: apitest.o
	$(CC) -o $@ $<
CLEANFILES += apitest apitest.o

alloctest: alloctest.o $(LIB)
	$(CC) -o $@ $< $(LIB)
CLEANFILES += alloctest alloctest.o

soltest: soltest.o $(LIB)
	$(CC) -o $@ $< $(LIB)
CLEANFILES += soltest soltest.o

lib:
	make -C ../src install

clean:
	$(call print,CLEAN)
	$(RM) $(CLEANFILES)
	make -C ../src clean

%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<
