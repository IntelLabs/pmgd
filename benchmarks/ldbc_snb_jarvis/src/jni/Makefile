LIB := libqueries-jni.so

QUERIES := 1 2 3 4 5 6 7 8 9 10 11 12 13 14

SRCS := $(QUERIES:%=jarvis_ldbc_JarvisDb_Query%.cc) common.cc
OBJS := $(SRCS:%.cc=%.o)
DEPS := $(OBJS:%.o=%.d)

JARVIS ?= /opt/intel/vdms/pmgd

LDLIBS := ../queries/queries.lib -L$(JARVIS)/lib -lpmgd-util -lpmgd

LDDEPS := $(filter-out -L%,\
          $(LDLIBS:-l%=$(subst -L,,$(filter -L%,$(LDLIBS)))/lib%.so))

JC := javac
JAVA_HOME := $(shell dirname $$(dirname $$(readlink -f $$(which $(JC)))))
JAVA_INCLUDES := -I $(JAVA_HOME)/include -I $(JAVA_HOME)/include/linux

CC := gcc

OPT ?= -O2

INCS := -I . -I include -iquote ../queries \
        -I $(JARVIS)/include -I $(JARVIS)/util \
        $(JAVA_INCLUDES)

CPPFLAGS := -std=c++11 -fPIC -MMD $(OPT) $(INCS)

default: all

all: $(LIB)

$(LIB) : $(OBJS) $(LDDEPS)
	$(CC) -shared -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f $(LIB) $(OBJS) $(DEPS)

-include $(DEPS)
