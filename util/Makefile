CC := g++-4.8
AR := ar
CP := cp -a
RM := rm -f
ECHO := echo

O := ..

TARGET := $O/util/jarvis-util.lib
INSTALLPATH := $O/lib
CLEANFILES :=

SRCS := exception.cc text.cc dump.cc load_tsv.cc load_gson.cc loader.y scanner.l

# Translates source file names to object file names
OBJS := $(patsubst %.cc,$O/util/%.o, \
          $(patsubst %.y,$O/util/%.o, \
            $(patsubst %.l,$O/util/%.o, $(SRCS))))

# Add the output of flex and bison to the list of files to clean
CLEANFILES += $(patsubst %.y,$O/util/%.cc, \
                $(patsubst %.l,$O/util/%.cc, \
                  $(filter %.y %.l, $(SRCS))))
CLEANFILES += $(patsubst %.y,$O/util/%.hh, \
                $(patsubst %.l,$O/util/%.hh, \
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

install: $O/util $(TARGET)
	$(call print,INSTALL,$(INSTALLPATH))
	$(shell if ! test -d $(INSTALLPATH); then mkdir $(INSTALLPATH); fi)
	$(CP) $(TARGET) $(INSTALLPATH)

$(TARGET): $(OBJS)
	$(call print,AR,$@)
	$(AR) cr $@ $(OBJS)

clean:
	$(call print,CLEAN)
	$(RM) $(TARGET) $(OBJS) $(DEPS) $(CLEANFILES)

$O/util:
	mkdir -p $@

$O/util/%.o: %.cc $(MAKEFILE_LIST)
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

$O/util/%.o: $O/util/%.cc
	$(call print,COMPILE,$@)
	$(CC) $(CFLAGS) -o $@ -c $<

$O/util/scanner.cc: scanner.l
	$(call print,LEX,$<)
	flex $(LFLAGS) -o$@ --header-file=$*.hh $<

$O/util/loader.cc: loader.y
	$(call print,YACC,$<)
	bison $(YFLAGS) -d -o $@ $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)
$O/util/loader.cc: $(MAKEFILE_LIST)
$O/util/scanner.cc: $(MAKEFILE_LIST)
