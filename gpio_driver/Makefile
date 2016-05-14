#
# my simple makefile act as something like a user interface
#

MODULES = $(shell ls -d */ | cut -f1 -d '/')
PWD=$(shell pwd)

all: driver

driver:
	@echo "+------------------------------------------------------------+"
	@echo "|   build $@ in $(PWD)"
	@echo "+------------------------------------------------------------+"
	(cd $@ && $(MAKE))

usage:
	@echo "+------------------------------------------------------------+"
	@echo "|   build $@ in $(PWD)"
	@echo "+------------------------------------------------------------+"

	(cd $@ && $(MAKE))

install:
	for dir in $(MODULES); do (cd $$dir && $(MAKE) $@); done

uninstall:
	for dir in $(MODULES); do (cd $$dir && $(MAKE) $@); done

clean:
	rm -f *~ .*~
	for dir in $(MODULES); do (cd $$dir && $(MAKE) $@); done

distclean: clean

.PHONY: clean driver usage install uninstall
