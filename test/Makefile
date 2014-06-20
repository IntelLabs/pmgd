CC := g++-4.8
RM := rm -f
ECHO := echo

O := ..

LIBS := $O/lib/jarvis-util.lib $O/lib/jarvis.lib
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

OBJS := $(TESTS:%=$O/test/%.o)
BINS := $(TESTS:%=$O/test/%)

CLEANFILES += $(OBJS) $(BINS)

all: lib util $O/test $(BINS)

$O/test/apitest: $O/test/apitest.o
	$(CC) -o $@ $<

$O/test/alloctest: $O/test/alloctest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$O/test/soltest: $O/test/soltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/nodeedgetest: $O/test/nodeedgetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/listtest: $O/test/listtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$O/test/edgeindextest: $O/test/edgeindextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$O/test/propertytest: $O/test/propertytest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/propertypredicatetest: $O/test/propertypredicatetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/propertychunktest: $O/test/propertychunktest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/propertylisttest: $O/test/propertylisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -static -o $@ $< $(LIBS)

$O/test/filtertest: $O/test/filtertest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/txtest: $O/test/txtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/stringtabletest: $O/test/stringtabletest.o $(LIBS)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/setproperty: $O/test/setproperty.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/load_tsv_test: $O/test/load_tsv_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$O/test/load_gson_test: $O/test/load_gson_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -ljsoncpp

$O/test/avltest: $O/test/avltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$O/test/chunklisttest: $O/test/chunklisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$O/test/indextest: $O/test/indextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

lib:
	make -C ../src O=$O OPT=$(OPT) install

util:
	make -C ../util O=$O OPT=$(OPT) install

$O/test:
	mkdir -p $@

$O/test/%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

CLEANFILES += $(DEPS)

clean:
	$(call print,CLEAN)
	$(RM) $(CLEANFILES)
	make -C ../src O=$O clean
	make -C ../util O=$O clean
