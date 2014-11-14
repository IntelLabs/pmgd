MAKE := make --no-print-directory
ECHO := echo

DIR := $(shell basename $$PWD)

.PHONY: default all clean cleanall src util tools test help

default: ; @$(MAKE) -C .. $(DIR)

all: ; @$(MAKE) -C .. all

clean: ; @$(ECHO) "Not implemented"

cleanall: ; @$(MAKE) -C .. clean

src: ; @$(MAKE) -C .. src

util: ; @$(MAKE) -C .. util

tools: ; @$(MAKE) -C .. tools

test: ; @$(MAKE) -C .. test

%:: force
	@$(MAKE) -C .. $(DIR)/$@

help:
	@$(ECHO) "make          - Build all, in current directory"
	@$(ECHO) "make clean    - Clean all, in current directory (not implemented)"
	@$(ECHO) "make all      - Build everything in the project"
	@$(ECHO) "make cleanall - Clean everything in the project"
	@$(ECHO) "make <dir>    - Build all in ../dir"
	@$(ECHO) "make <target> - Build specified target only, in current directory"

Makefile: ;

.PHONY: force
force:
