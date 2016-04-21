My template drivers
===================

This is a collection of different linux kernel driver templates:

  an absolute minimal driver
  char driver
  i2c driver
  gpio driver
  spi driver


minimal driver
--------------

The purpose for this driver is simply to check an crosstool enviroment. If you work with embedded device you often need to crossbuild drivers. With that simple driver you can check if your toolchain works and if you have the correct versions.
Simply crossbuild it vi make ARCH=... CROSS_COMPILE=... KDIR=... and transfer minimal_driver.ko to your device and then insmod it, check syslog entry and rmmod it. If everthing works fine, then your environment should be also fine, otherwise you have to check.


char driver
-----------

Basic character driver with open, close, read and write support. It uses device numbers instead of major/minor.


char driver old school
----------------------

Basic character driver with open, close, read and write support. It uses major/minor number instead device numbers. This is the old way, usage devices numbers are preferred.


i2c driver
----------

Basic for drivers like https://github.com/tjohann/lcd160x_driver.git


gpio driver
-----------

Basic for drivers like https://github.com/tjohann/pcf8574_gpio.git


spi driver
----------

Basic for drivers like https://github.com/tjohann/max7119_array.git


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