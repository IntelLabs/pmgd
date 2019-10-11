LIB := queries.lib

QUERIES := query1 query2 query3 query4 query5 query6 query7 query8 query9 \
           query10 query11 query12 query13 query14

OBJS := $(QUERIES:%=%.o)
DEPS := $(OBJS:%.o=%.d) $(QUERIES:%=%+.d)

JARVIS ?= /opt/intel/vdms/pmgd
LDLIBS := $(JARVIS)/lib/pmgd-util.lib $(JARVIS)/lib/pmgd.lib

CC := g++

OPT ?= -O2

INCS := -I . -I $(JARVIS)/include -I $(JARVIS)/util

CPPFLAGS := -std=c++11 -fPIC -MMD $(OPT) $(INCS)

default: $(LIB)

all: $(QUERIES) $(LIB)

$(LIB) : $(OBJS)
	$(AR) cr $(LIB) $(OBJS)

query%.o : query%.cc
	$(CC) $(CPPFLAGS) -c $<

query% : query%.cc
	$(CC) -DMAIN $(CPPFLAGS) -MF $@+.d -o $@ $< $(LDLIBS)

clean:
	rm -f $(LIB) $(OBJS) $(DEPS) $(QUERIES)

-include $(DEPS)
