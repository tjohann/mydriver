My template drivers
===================

This is a collection of different linux kernel driver templates:

  an absolute minimal driver
  char driver
  i2c driver
  gpio driver
  spi driver

It's an playground for different topics. So you find also schematics and pics about my test setup but be aware: this is work in progress! Don't expect things to be complete in any dimension! 


minimal driver
--------------

The purpose for this driver is simply to check an crosstool enviroment. If you work with embedded device you often need to crossbuild drivers. With that simple driver you can check if your toolchain works and if you have the correct versions.
Simply crossbuild it vi make ARCH=... CROSS_COMPILE=... KDIR=... and transfer minimal_driver.ko to your device and then insmod it, check syslog entry and rmmod it. If everthing works fine, then your environment should be also fine, otherwise you have to check.

State: finished


char driver
-----------

Basic character driver with open, close, read and write support. It uses device numbers instead of major/minor.

State: nearly finished (userspace example missing)


char driver old school
----------------------

Basic character driver with open, close, read and write support. It uses major/minor number instead device numbers. This is the old way, usage devices numbers are preferred.

State: started


i2c driver
----------

Basic for drivers like https://github.com/tjohann/lcd160x_driver.git

State: not started


gpio driver
-----------

Basic for drivers like https://github.com/tjohann/pcf8574_gpio.git

State: started


spi driver
----------

Basic for drivers like https://github.com/tjohann/max7119_array.git

State: not started


common structure
----------------

Nearly all drivers in this repository have the same structure:

|-- char_driver
|   |-- driver
|   |   |-- char_driver.c
|   |   |-- ...
|   |   `-- Makefile
|   |-- ... 
|   |-- TODO
|   `-- usage
|       |-- usage.c
|       |-- ...
|       `-- Makefile
|
...


Below the usage directory you can find one or more examples on how to access the driver. 


userspace examples
------------------

Below the directory userspace_examples you find my basic userspace playground. To implement an I2C driver you normally also have to implement the protocol which is specific to the IC and it's functionlity. As example think of the LCD1602 connected via PCF8574 portexpander with the I2C bus.

State (pcf8574_usage): started


Documentation
-------------

Below the directory Documentation you can find useful information about the used IC or the protocol or ... take a look at it.


pics
----

Here you find some pictures of the wiring and my test setup. 


schematics
----------

Here you find some simple schematics of my test setup.


