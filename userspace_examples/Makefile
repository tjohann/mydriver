#
# my simple makefile act as something like a user interface
#

MODULES = $(shell ls -d */ | cut -f1 -d '/')
PWD=$(shell pwd)

all: pcf8574_cyclon pcf8574_input pcf8574_lcd1602

pcf8574_cyclon:
	@echo "+------------------------------------------------------------+"
	@echo "|   build $@ in $(PWD)"
	@echo "+------------------------------------------------------------+"
	(cd $@ && $(MAKE))

pcf8574_input:
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

.PHONY: uninstall install clean pcf8574_cyclon pcf8574_input
