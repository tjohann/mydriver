#
# my simple makefile act as something like a user interface
#

CLEAN = $(shell ls -d */ | cut -f1 -d'/')
BUILD = char_driver 		\
	i2c_gpio_driver 	\
	gpio_driver 		\
	gpio_driver_new		\
	gpio_irq_driver 	\
	gpio_irq_driver_new 	\
	spi_driver 		\
	userspace_examples

all:
	@echo "  +----------------------------------------------------------+"
	@echo " /                                                          /|"
	@echo "+----------------------------------------------------------+ |"
	@echo "| Usage:                                                   | |"
	@echo "| make build     -> build everthing                        | |"
	@echo "| make install   -> build_all + install (driver + examples)| |"
	@echo "| make uninstall -> uninstall all                          | |"
	@echo "| make clean     -> clean all dir/subdirs                  | +"
	@echo "| make distclean -> complete cleanup                       |/ "
	@echo "+----------------------------------------------------------+  "

build:
	for dir in $(BUILD); do (cd $$dir && $(MAKE) all); done

clean:
	rm -f *~ .*~
	for dir in $(CLEAN); do (cd $$dir && $(MAKE) $@); done

install:
	for dir in $(BUILD); do (cd $$dir && $(MAKE) $@); done

uninstall:
	for dir in $(BUILD); do (cd $$dir && $(MAKE) $@); done

distclean:
	rm -f *~ .*~
	for dir in $(CLEAN); do (cd $$dir && $(MAKE) $@); done

.PHONY: all build clean distclean install uninstall
