CC := g++-4.8
RM := rm -f
ECHO := echo

ifeq ($O,)
    O_ROOT := ../
    O_TEST :=
    O_LIB  := ../lib/
else
    O_ROOT := $O/
    O_TEST := $O/test/
    O_LIB  := $O/lib/
endif

LIBS := $(O_LIB)jarvis-util.lib $(O_LIB)jarvis.lib
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
         avltest chunklisttest indextest indexstringtest

OBJS := $(TESTS:%=$(O_TEST)%.o)
BINS := $(TESTS:%=$(O_TEST)%)

CLEANFILES += $(OBJS) $(BINS)

all: lib util $(O_TEST) $(BINS)

$(O_TEST)apitest: $(O_TEST)apitest.o
	$(CC) -o $@ $<

$(O_TEST)alloctest: $(O_TEST)alloctest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)soltest: $(O_TEST)soltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)nodeedgetest: $(O_TEST)nodeedgetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)listtest: $(O_TEST)listtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)edgeindextest: $(O_TEST)edgeindextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)propertytest: $(O_TEST)propertytest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)propertypredicatetest: $(O_TEST)propertypredicatetest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)propertychunktest: $(O_TEST)propertychunktest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)propertylisttest: $(O_TEST)propertylisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -static -o $@ $< $(LIBS)

$(O_TEST)filtertest: $(O_TEST)filtertest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)txtest: $(O_TEST)txtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)stringtabletest: $(O_TEST)stringtabletest.o $(LIBS)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)setproperty: $(O_TEST)setproperty.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)load_tsv_test: $(O_TEST)load_tsv_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS)

$(O_TEST)load_gson_test: $(O_TEST)load_gson_test.o $(LIBS) util
	$(call print,LNK,$@)
	$(CC) $(OPT) -o $@ $< $(LIBS) -ljsoncpp

$(O_TEST)avltest: $(O_TEST)avltest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)chunklisttest: $(O_TEST)chunklisttest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)indextest: $(O_TEST)indextest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

$(O_TEST)indexstringtest: $(O_TEST)indexstringtest.o $(LIBS)
	$(call print,LINK,$@)
	$(CC) -o $@ $< $(LIBS)

lib:
	make -C ../src O=$O OPT="$(OPT)" install

util:
	make -C ../util O=$O OPT="$(OPT)" install

$(O_TEST):
	mkdir -p $@

$(O_TEST)%.o: %.cc $(MAKEFILE_LIST)
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
