CC := g++-4.8
AR := ar
CP := cp -a
RM := rm -f
ECHO := echo

TARGET := jarvis-util.lib
INSTALLPATH := ../lib
CLEANFILES := 

SRC := exception.cc text.cc dump.cc load_tsv.cc load_gson.cc loader.y scanner.l

# Translates source file names to object file names
OBJS := $(patsubst %.cc,%.o, $(patsubst %.y,%.o, $(patsubst %.l,%.o, $(SRC))))

INCLUDES := -I../include
OPT ?= -O2

FFLAGS := -fomit-frame-pointer -funit-at-a-time -finline-limit=2000000 \
          -fno-strict-aliasing -fno-rtti -fno-threadsafe-statics

WFLAGS := -Wall -Wpointer-arith -Wcast-align -Wwrite-strings \
          -Wformat=2 -Wno-parentheses -Wno-conversion

CFLAGS := --std=c++11 $(INCLUDES) $(OPT) $(FFLAGS) $(WFLAGS) -MP -MMD

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

scanner.cc: scanner.l
	$(call print,LEX,$<)
	flex $(LFLAGS) -o$@ --header-file=$*.hh $<

loader.cc: loader.y
	$(call print,YACC,$<)
	bison $(YFLAGS) -d -o $@ $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
loader.cc scanner.cc: Makefile
