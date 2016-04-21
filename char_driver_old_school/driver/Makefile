# Makefile for char_driver_old_school.c
#
# GPL
# (c) 2016, thorsten.johannvorderbrueggen@t-online.de
# 

ifneq ($(KERNELRELEASE),)
obj-m	:= char_driver_old_school.o
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
