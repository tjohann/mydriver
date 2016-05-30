My template drivers
===================

This is a collection of different linux kernel driver templates:

1. an absolute minimal driver
2. char driver (use ioctl and write to define a new read buffer)
3. i2c-gpio driver_simple (PCF8574 based)
4. i2c-gpio driver (PCF8574 based with check of IRQ line)
5. gpio irq driver_simple (bind a PIN to an irq)
6. gpio irq driver (use write to define PIN)
7. gpio driver (use ioctl to define PIN for read/write)
8. spi driver (MAX7119 based)

It's an playground for different topics like I2C. Therefore i implement a userspace example based on what is already available within the kernel/userspace (like i2c-tools) and a driver with a specialized interface (and a example of how to use it). You find also schematics and pics about my test setup.

But be aware: this is work in progress! Don't expect things to be complete in any dimension! See the State info below to get an idea of what is working.


Usage
-----

The simple user interface to this repository is a Makefile. So type

    make

to get more info.


(Un)Installation
----------------

You can install all driver and userspace examples via

    make install


To get rid of them use

   make uninstall


The make install tag will install all modules to /lib/modules/$(shell uname -r)/kernel. So you can simple type

    modprobe char_driver

to load it. Depending on your distribution you can see info message from the driver in /var/log/messages.

   (sudo) tail -n 50 -f /var/log/messages

The userspace example (usage_$(DRIVER_NAME)) will be installed in $(HOME)/bin. If you have it in your $PATH, then you can simply tape

    usage_char_driver -a

and see in /var/log/messages what the driver is doing.


The different driver
--------------------

Every driver tries to implement a single topic.

Nearly all drivers in this repository have the same structure:

![Alt text](pics/driver_dir_tree.png?raw=true "Driver tree structure")

Below the usage directory you can find one or more examples on how to access the driver.


The minimal driver
------------------

The purpose for this driver is simply to check an crosstool enviroment. If you work with embedded device you often need to crossbuild drivers. With that simple driver you can check if your toolchain works and if you have the correct versions.
Simply crossbuild it vi make ARCH=... CROSS_COMPILE=... KDIR=... and transfer minimal_driver.ko to your device and then insmod it, check syslog entry and rmmod it. If everthing works fine, then your environment should be also fine, otherwise you have to check.

State: finished


The char driver
---------------

Basic character driver with open, close, read, write and ioctl support (PM-support prepared).

The driver develops a simple char_driver. It creates an dev-node (/dev/char_driver) on which you can use read, write and ioctl on it. The simple example shows how to use the driver:

1. open char_driver
2. read driver default string from driver
3. write TO_WRITE to driver
4. read new string from driver
5. set IOCTL_TO_WRITE via ioctl
6. read new string from driver

Build and load the driver:

![Alt text](Documentation/char_driver_build_load.png?raw=true "Build and load driver")

Example usage of the driver:

![Alt text](Documentation/char_driver_usage.png?raw=true "Usage of driver")

State: finished


The i2c-gpio driver (simple)
----------------------------

Simple driver to demonstrate the usage of an PCF8574 controlled via I2C.

Used hardware: Bananapi-M1

State: started


The i2c-gpio driver
------------------

Basic for drivers like https://github.com/tjohann/pcf8574_gpio.git and https://github.com/tjohann/lcd160x_driver.git

Feartures:
	Dts config for intr, id and more
	Intr handler for PIN13/IO-0/PI18 (IRQ line of PCF8574)
	Read/write all pins (8 bit)
	Using offset to read/write a bit position
	Controll mapping of A20-GPIO to PCF8574-IRQ-line via ioctl

Used defaults:

2. PIN13 (IO-0/PI18) for input

Used hardware: Bananapi-M1

State: not started


The gpio irq driver (simple)
----------------------------

Simple(st) driver to show the usage of an IRQ connected PIN (PIN13/IO-0/PI18 -> bananapi-m1).

Used defaults:

1. PIN16 (IO-4/PH20) for input

The driver doesn't support more than one instance and it uses only the default PIN.

State: finished


The gpio irq driver
-------------------

An int based driver to show the usage of an IRQ connected PIN. Via write syscall you have to define a input PIN, otherwise it will use the defaults.

Used hardware: Olimex-A20-SOM-EVB

Usage:

	usage_gpio_irq_driver (read from default PIN PH20/244)
	usage_gpio_irq_driver -p 123 (read from to PIN 123)

Example usage of the driver (with 3/4 instance) using PH8, PH16, PH20 and PH21:

![Alt text](pics/gpio_irq_driver_usage_01.png?raw=true "Usage of driver -> start 4 instance")

![Alt text](pics/gpio_irq_driver_usage_02.png?raw=true "Usage of driver -> 4 instance in action")

![Alt text](pics/gpio_irq_driver_usage_03.png?raw=true "Usage of driver -> 3 instance with /proc/interrupts")

![Alt text](pics/gpio_irq_driver_usage_04.png?raw=true "Usage of driver -> 3 instance with ps aux")

State: finished


The gpio irq driver (new)
-------------------------

Simple (fd based) driver to show the usage of an IRQ connected PIN (PIN13/IO-0/PI18). It behaves in the same way like gpio_irq_driver, but it uses the new file descriptor gpio framework of the kernel

State: not started


The gpio driver
---------------

Simple (int based) driver to show the usage of gpio for read and write from a PIN. Via ioctl syscall you can change the PIN for in/output. It doesn't bind a PIN to an IRQ!

Used defaults:

1. PIN11 (IO-0/PI19) for output
2. PIN13 (IO-0/PI18) for input

Used hardware: Bananapi-M1

Usage:

	usage_gpio_driver -r (read from default pin)
	usage_gpio_driver -w (write to default pin)
	usage_gpio_driver -rp 321 (read from pin 321)
	usage_gpio_driver -w -p 123 (write to pin 123)

Note: The driver configured the default pin after open. So to use more than one instance you have to use ioctl to change the pin.

Example usage of the driver:

![Alt text](pics/gpio_driver_02.png?raw=true "Gpio-driver in action")

State: finished


The gpio driver (new)
---------------------

Simple (fd based) driver to show the usage of gpio for read and write from a PIN. It behaves in the same way like gpio_driver, but it uses the new file descriptor gpio framework of the kernel.

State: not started


The spi driver
--------------

Basic for drivers like https://github.com/tjohann/max7119_array.git

State: not started


The userspace examples
----------------------

Below the directory userspace_examples you find my basic userspace playground. In most cases the userspace parts are the starting point for the driver develpment and the functionality provided by "normal" userspace will be implemented within a special driver(s). They are the basic for my blog entrys about embedded linux (https://tjohann.wordpress.com/category/embedded-realtime/) and linux realtime topics (https://github.com/tjohann/time_triggert_env.git).


The userspace examples (pcf8574)
--------------------------------------

To implement an I2C driver you normally also have to implement the protocol which is specific to the IC and it's functionlity. As example think of the LCD1602 connected via PCF8574 portexpander with the I2C bus.
All example use the i2c-dev driver (http://lxr.free-electrons.com/source/Documentation/i2c/dev-interface) and i2c-tools (www.lm-sensors.org).

State (pcf8574): started


The userspace examples (pcf8574_hd44780)
----------------------------------------

Based on the pcf8574 this example shows the usage with an LCD1602 display.

State (pcf8574_hd44780): not started


The userspace examples (gpio_script)
------------------------------------

This is a simple demonstration about the usage of sysfs-gpio interface. This scripts toogle PIN11(IO-0/PI19) of a bananapi. See ./Documentation/gpio_bananapi.txt about calculation gpio value and PIN.

Another script reads PIN13(IO-2/PI18) and switch PIN11(IO-0/PI19) on/off.

State (gpio_script): finished


Documentation
-------------

Below the directory Documentation you can find useful information about the used IC or the protocol or ... take a look at it.


Pictures/Test-environment
-------------------------

Here you find some pictures of the wiring and my test setup.

The gpio test environment:

![Alt text](pics/overview_06.jpg?raw=true "Overview of GPIO-Environment")

![Alt text](pics/gpio_led_04.jpg?raw=true "GPIO-LED and switch")


The gpio IRQ test environment:

![Alt text](pics/gpio_irq_input_env_02.jpg?raw=true "GPIO-Switches")

![Alt text](pics/gpio_input_02.jpg?raw=true "GPIO-Switches")


Schematics
----------

Here you find some simple schematics of my test setup.


