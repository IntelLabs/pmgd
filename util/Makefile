CC := g++-4.8
AR := ar
CP := cp -a
RM := rm -f
ECHO := echo

ifeq ($O,)
    O_ROOT := ../
    O_UTIL :=
else
    O_ROOT := $O/
    O_UTIL := $O/util/
endif

TARGET := $(O_UTIL)jarvis-util.lib
INSTALLPATH := $(O_ROOT)lib
CLEANFILES :=

SRCS := exception.cc text.cc dump.cc load_tsv.cc load_gson.cc loader.y scanner.l

# Translates source file names to object file names
OBJS := $(patsubst %.cc,$(O_UTIL)%.o, \
          $(patsubst %.y,$(O_UTIL)%.o, \
            $(patsubst %.l,$(O_UTIL)%.o, $(SRCS))))

# Add the output of flex and bison to the list of files to clean
CLEANFILES += $(patsubst %.y,$(O_UTIL)%.cc, \
                $(patsubst %.l,$(O_UTIL)%.cc, \
                  $(filter %.y %.l, $(SRCS))))
CLEANFILES += $(patsubst %.y,$(O_UTIL)%.hh, \
                $(patsubst %.l,$(O_UTIL)%.hh, \
                  $(filter %.y %.l, $(SRCS))))

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

install: $(O_UTIL) $(TARGET)
	$(call print,INSTALL,$(INSTALLPATH))
	$(shell if ! test -d $(INSTALLPATH); then mkdir $(INSTALLPATH); fi)
	$(CP) $(TARGET) $(INSTALLPATH)

$(TARGET): $(OBJS)
	$(call print,AR,$@)
	$(AR) cr $@ $(OBJS)

clean:
	$(call print,CLEAN)
	$(RM) $(TARGET) $(OBJS) $(DEPS) $(CLEANFILES)

$(O_UTIL):
	mkdir -p $@

$(O_UTIL)%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(O_UTIL)%.o: $(O_UTIL)%.cc
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(O_UTIL)scanner.cc: scanner.l
	$(call print,LEX,$<)
	flex $(LFLAGS) -o$@ --header-file=$*.hh $<

$(O_UTIL)loader.cc: loader.y
	$(call print,YACC,$<)
	bison $(YFLAGS) -d -o $@ $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
$(O_UTIL)loader.cc: $(MAKEFILE_LIST)
$(O_UTIL)scanner.cc: $(MAKEFILE_LIST)
