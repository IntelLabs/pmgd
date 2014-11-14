##
# Main Makefile for the Jarvis Lake library.

# make must forget all it knows about suffixes.
.SUFFIXES:

# Tools.
MAKE := make --no-print-directory -s -k -j8
RM := rm -f
MKDIR := mkdir -p
RMDIR := rmdir

ifeq ($O,)
# Local build.
.PHONY: all
all:
	+@$(MAKE) -f Makeconf

%: force
	+@$(MAKE) -f Makeconf $@

.PHONY: force
force:
else
# Remote build.
.PHONY: all
all: $O
	+@$(MAKE) -C $O -f $(CURDIR)/Makeconf ROOTDIR=$(CURDIR)

%: $O
	+@$(MAKE) -C $O -f $(CURDIR)/Makeconf ROOTDIR=$(CURDIR) $@

clean:
	@$(MAKE) -C $O -f $(CURDIR)/Makeconf ROOTDIR=$(CURDIR) clean
	@$(RMDIR) $O/src $O/util $O/tools $O/test $O

.PHONY: $O
$O:
	@$(MKDIR) $@/src $@/util $@/tools $@/test
endif

# Don't attempt to rebuild this Makefile.
Makefile : ;
