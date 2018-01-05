/*
 * pcf8574_lcd1602.c -> show userspace usage of i2c-dev
 *
 * GPL
 * (c) 2017, thorsten.johannvorderbrueggen@t-online.de
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ADAPTER_LEN 19
#define PCF8474_RW_MODE 0x00


/*
  Pinning LCDxxxx to PCF8574

  - rs  -> pin1
  - rw  -> pin2
  - en  -> pin3
  - bl  -> pin4  (backlight)
  - db4 -> pin5
  - db5 -> pin6
  - db6 -> pin7
  - db7 -> pin8
*/
#define LCD_RS    0
#define LCD_RW    1
#define LCD_ENA   2
#define LCD_BL    3

#define WRITE_DATA() do {					\
		if (write(fd, data, 2) != 2) {			\
			perror("write");			\
			return -1;				\
		}						\
	} while(0)


#define LCD_EN_TO_HIGH() do {		\
		*ptr |= (1 << LCD_ENA);	\
	} while(0)

#define LCD_EN_TO_LOW() do {		\
		*ptr &= ~(1 << LCD_ENA);\
	} while(0)

#define LCD_SET_RS_TO_COMMAND() do {	\
		*ptr &= ~(1 << LCD_RS);	\
	} while(0)

#define LCD_SET_RS_TO_CHARACTER() do {	\
		*ptr |= (1 << LCD_RS);	\
	} while (0)

#define LCD_SET_RW_TO_WRITE() do {	\
		*ptr &= ~(1 << LCD_RW);	\
	} while(0)

#define LCD_SET_RW_TO_READ() do {	\
		*ptr |= (1 << LCD_RW);	\
	} while (0)

#define LCD_SET_BACKLIGHT_ON() do {	\
		*ptr |= (1 << LCD_BL);	\
	} while (0)

#define LCD_SET_BACKLIGHT_OFF() do {	\
		*ptr |= (1 << LCD_BL);	\
	} while (0)


static void
__attribute__((noreturn)) usage(void)
{
        fprintf(stdout, "Usage: ./usage [0...9] 0x[XX] [0 ... 255] \n");
        fprintf(stdout, "       1  -> adapter 1 (/dev/i2c-1)       \n");
        fprintf(stdout, "       0x22 -> i2c address 0x22           \n");

        exit(EXIT_FAILURE);
}

static int
send_data(int fd, unsigned char *data, bool is_character)
{
	unsigned char *ptr = &data[1];

	unsigned char tmp = data[1];
	fprintf(stdout, "data to send 0x%.2x\n", tmp);

	*ptr = 0x00;
	*ptr = (tmp & 0xF0);
	printf("nach shift >> 4 ... value: %d\n", *ptr);
	if (is_character)
		LCD_SET_RS_TO_CHARACTER();

	LCD_SET_BACKLIGHT_ON();
	LCD_EN_TO_HIGH();
	printf("send data ... value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);

	LCD_EN_TO_LOW();
	printf("send data ... value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);

	*ptr = 0x00;
	*ptr =((tmp & 0x0F) << 4);
	printf("nun das zweite nibble ... value: %d\n", *ptr);

	LCD_SET_BACKLIGHT_ON();
	LCD_EN_TO_HIGH();
	printf("send data ... value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);

	LCD_EN_TO_LOW();
	printf("send data ... value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);

	printf("\n");

	return 0;
}

static unsigned char
create_addr_byte(unsigned char addr)
{
	/* rw = 0 -> write, rw = 1 -> read */
	unsigned char rw = PCF8474_RW_MODE;

	unsigned char addr_byte = addr << 1;
	addr_byte |= rw;

	return addr_byte;
}

/*
  Pinning LCD1602 to PCF8574

  - rs  -> pin1
  - rw  -> pin2
  - en  -> pin3
  - bl  -> pin4  (backlight)
  - db4 -> pin5
  - db5 -> pin6
  - db6 -> pin7
  - db7 -> pin8
*/
static int
init_i2c_lcd(int fd, unsigned char *data)
{
	fprintf(stdout, "will init LCD with address byte 0x%x\n", data[0]);

	unsigned char *ptr = &data[1];

	*ptr = 0x00;
	*ptr = (0x03 << 4);
	LCD_SET_RS_TO_COMMAND();
	LCD_SET_BACKLIGHT_ON();

	printf("value: %d\n", *ptr);

	LCD_EN_TO_HIGH();
	printf("value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);
	LCD_EN_TO_LOW();
	printf("value: %d\n", *ptr);
	WRITE_DATA();
	usleep(4100);

	LCD_EN_TO_HIGH();
	printf("send data ... value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);
	LCD_EN_TO_LOW();
	printf("send data ... value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);

	LCD_EN_TO_HIGH();
	WRITE_DATA();
	printf("send data ... value: %d\n", *ptr);
	usleep(100);
	LCD_EN_TO_LOW();
	WRITE_DATA();
	printf("send data ... value: %d\n", *ptr);
	usleep(4100);

	printf("erster Teil init zu ende\n");

	*ptr = 0x20;
	LCD_SET_RS_TO_COMMAND();
	LCD_SET_BACKLIGHT_ON();

	LCD_EN_TO_HIGH();
	printf("value: %d\n", *ptr);
	WRITE_DATA();
	usleep(100);
	LCD_EN_TO_LOW();
	printf("value: %d\n", *ptr);
	WRITE_DATA();
	usleep(4100);

	printf("zweiter Teil init zu ende\n\n");

	*ptr = 0x28;
	send_data(fd, data, false);

	*ptr = 0x08;
	send_data(fd, data, false);

	*ptr = 0x01;
	send_data(fd, data, false);

	*ptr = 0x06;
	send_data(fd, data, false);

	/* my stuff */
//	*ptr = 0x0f;
//	send_data(fd, data, false);

	return 0;
}


int
main(int argc, char *argv[])
{
	int fd = -1;
	char adapter_s[MAX_ADAPTER_LEN + 1];
	memset(adapter_s, 0, sizeof(adapter_s));

	if (argc != 3)
		usage();

	unsigned char adapter_nr = atoi(argv[1]);
	unsigned char addr = strtoul(argv[2], NULL, 0);

	snprintf(adapter_s, MAX_ADAPTER_LEN, "/dev/i2c-%d", adapter_nr);
	fprintf(stdout, "try to open %s@0x%x\n", adapter_s, addr);

	fd = open(adapter_s, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}

	unsigned char data[2];
	memset(data, 0, sizeof(data));

	data[0] = create_addr_byte(addr);
	fprintf(stdout, "addr_byte = 0x%x\n", data[0]);

	if (init_i2c_lcd(fd, data) == -1) {
		fprintf(stderr, "could not init i2c lcd\n");
		exit(EXIT_FAILURE);
	}

	/* the main loop */



	/* end of main loop */
	sleep(5);

	if (fd > 0)
		close(fd);

	return EXIT_SUCCESS;
}
