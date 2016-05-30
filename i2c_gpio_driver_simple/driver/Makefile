# Makefile for gpio_driver.c
#
# GPL
# (c) 2016, thorsten.johannvorderbrueggen@t-online.de
#

DRIVER=i2c_gpio_driver_simple

ifneq ($(KERNELRELEASE),)
obj-m	:= $(DRIVER).o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE)	-C $(KDIR) M=$(PWD) modules
endif

clean:
	rm -f *~ *.o *.ko
	rm -f .built_in.o.cmd built_in.o
	rm -f Module.symvers modules.order
	rm -f .*.cmd *.ko *.mod.c
	rm -rf .tmp_versions

install: default
	sudo cp $(DRIVER).ko /lib/modules/$(shell uname -r)/kernel
	sudo depmod -a

uninstall:
	sudo rm -rf /lib/modules/$(shell uname -r)/kernel/$(DRIVER).ko
	sudo depmod -a

.PHONY: clean uninstall
