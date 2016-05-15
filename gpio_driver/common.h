/*
 * char_driver.c -> simple template driver
 *
 * GPL
 * (c) 2013-2016, thorsten.johannvorderbrueggen@t-online.de
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


struct _gpio_pin {
	int pin;
	char *name;
	bool used;
};

/* LED -> IN11(IO-0/PI19) -> 275 */
struct _gpio_pin pin_write = {
	.pin = 275,
	.name = "gpio_write",
	.used = false
};

/* SWT -> PIN13(IO-2/PI18) -> 274 */
struct _gpio_pin pin_read = {
	.pin = 274,
	.name = "gpio_read",
	.used = false
};

#endif
