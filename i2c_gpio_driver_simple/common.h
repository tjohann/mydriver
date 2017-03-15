/*
 * i2c_gpio_driver_simple.c -> simple template driver
 *
 * GPL
 * (c) 2013-2017, thorsten.johannvorderbrueggen@t-online.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#define DRIVER_NAME "pcf8574_simple"
#define DEV_NAME "/dev/pcf8574_simple"

#define IOCTL_SET_I2C_CLIENT 0x0001

#define PIN_1 0x01
#define PIN_2 0x02
#define PIN_3 0x04
#define PIN_4 0x08
#define PIN_5 0x10
#define PIN_6 0x20
#define PIN_7 0x40
#define PIN_8 0x80

struct client_config {
	int adapter_nr;
	unsigned short addr;
};

#endif
