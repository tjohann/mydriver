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

/* define bit positions to be used below */

enum bit_pos_priv {
	BIT0 = 1 << 0,
	BIT1 = 1 << 1,
	BIT2 = 1 << 2,
	BIT3 = 1 << 3,
	BIT4 = 1 << 4,
	BIT5 = 1 << 5,
	BIT6 = 1 << 6,
	BIT7 = 1 << 7
};

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

  Note: unsigned char ctrl_nibble for pin1 -> pin4
        unsigned char *ptr for the whole byte to send via i2c
	unsigned char low_nibble are the bits 0 -> 3 of *ptr
	unsigned char high_nibble are the bits 4 -> 7 of *ptr
*/
#define LCD_RS    0
#define LCD_RW    1
#define LCD_ENA   2
#define LCD_BL    3

/*
 * Instructions
 */
#define CLEAR_DISPLAY       BIT0
#define RETURN_HOME         BIT1
#define ENTRY_MODE_SET      BIT2
#define DISPLAY_ON_OFF_CTRL BIT3
#define CURSOR_SHIFT        BIT4
#define FUNCTION_SET        BIT5
#define CGRAM_ADDR          BIT6
#define DDRAM_ADDR          BIT7

/*
 * Flags for instructions
 */
#define FIVE_X_TEN_FONTS    BIT2
#define TWO_LINES           BIT3
#define DATALENGTH_8BIT     BIT4
#define INC_DDRAM_ADDR      BIT1
#define CURSOR_BLINK        BIT0
#define CURSOR_ON           BIT1
#define DISPLAY_ON          BIT2


int init_i2c_lcd(int fd, unsigned char *data);
int config_lcd(int fd, unsigned char *data);
int enable_lcd(int fd, unsigned char *data);

/*
 * ----------------------------- other stuff -----------------------------------
 */

static void
__attribute__((noreturn)) usage(void)
{
        fprintf(stdout, "Usage: ./usage [0...9] 0x[XX] [0 ... 255] \n");
        fprintf(stdout, "       1  -> adapter 1 (/dev/i2c-1)       \n");
        fprintf(stdout, "       0x22 -> i2c address 0x22           \n");

        exit(EXIT_FAILURE);
}

static int
init_lcd(int fd, unsigned char *data)
{
        /*
	 * the init of the lcd is splitted into 3 parts
	 * - basic setup of the display (reset  and set to 4bit mode
	 * - clear and configure the lcd (2-lines ...)
	 * - enable lcd
	 */
	if (init_i2c_lcd(fd, data) == -1) {
		fprintf(stderr, "could not init i2c lcd\n");
		return -1;
	}

	if (config_lcd(fd, data) == -1){
		fprintf(stderr, "could not configure lcd\n");
		return -1;
	}

	if (enable_lcd(fd, data) == -1){
		fprintf(stderr, "could not configure lcd\n");
		return -1;
	}

	return 0;
}


/*
 * ----------------------------- I2C specific ----------------------------------
 */

/*
 * create the address byte data[0] ... data[1] is for the data to send
 */
unsigned char
create_addr_byte(unsigned char addr)
{
	/* rw = 0 -> write, rw = 1 -> read */
	unsigned char rw = PCF8474_RW_MODE;

	unsigned char addr_byte = addr << 1;
	addr_byte |= rw;

	return addr_byte;
}

/*
 * send the data via i2c connection to lcd
 *
 * TODO: send_nibble_data -> 4 bit mode (direct conected)
 *       send_byte_data   -> 8 bit mode (direct conected)
 */
int
send_i2c_data(int fd, unsigned char *data)
{
	/*
	 * see hitachi hd44780U datasheet at side 49ff
	 */

	unsigned char *ptr = &data[1];

	if (write(fd, data, 2) != 2) {
		perror("write");
		return -1;
	}

	int err = usleep(1); /* address setup tim > 140ns*/
	if (err == -1)
		return -1;

	*ptr |= (1 << LCD_ENA);
	if (write(fd, data, 2) != 2) {
		perror("write");
		return -1;
	}

	err = usleep(1); /* enable time > 450ns*/
	if (err == -1)
		return -1;

	*ptr &= ~(1 << LCD_ENA);
	if (write(fd, data, 2) != 2) {
		perror("write");
		return -1;
	}
	err = usleep(1);   /* data/address hold time > 20ns*/
	if (err == -1)
		return -1;

	return 0;
}

int
init_i2c_lcd(int fd, unsigned char *data)
{
	/*
	 * see hitachi hd44780U datasheet at side 24
	 *
	 * wait > 15ms
	 * RS and R/W set to 0
	 * DB4 and DB5 set to 1 (0x03)
	 * send this 3 times with 4100ms waittime after first try
	 * set lcd to 4 bit mode (0x02)
	 *
	 */
	unsigned char *ptr = &data[1];

	/* set ctrl bits (pin1 -> pin4) */
	unsigned char ctrl_nibble = (1 << LCD_BL);

	/* instruction: function set -> 001(1) 8bit mode */
	unsigned char high_nibble = (0x03 << 4);

	/* data to send */
	*ptr = ctrl_nibble | high_nibble;

	int err = usleep(40);
	if (err == -1)
		return -1;

	for (int i = 0; i < 3; i++) {
		err = send_i2c_data(fd, data);
		if (err == -1)
			return -1;

		err = usleep(4100);
		if (err == -1)
			return -1;
	}

	/* instruction: function set -> 001(0) 4bit mode */
	high_nibble = (0x02 << 4);

	/* data to send */
	*ptr = ctrl_nibble | high_nibble;

	err = send_i2c_data(fd, data);
	if (err == -1)
		return -1;

	err = usleep(1600);
	if (err == -1)
		return -1;

	return 0;
}

/*
 * ----------------------------- common layer ----------------------------------
 */

/*
 * send the data for 4 bit mode via i2c
 *
 * TODO: make configurable for 4 bit mode (direct conected)
 *       make configurable for 8 bit mode (direct conected)
 */
int
send_data(int fd, unsigned char *data, bool is_character)
{
	unsigned char *ptr = &data[1];

	unsigned char ctrl_nibble = (1 << LCD_BL);
	if (is_character)
		ctrl_nibble |= (1 << LCD_RS);
	else
		ctrl_nibble &= ~(1 << LCD_RS);

	/* first send the hight nibble, then the low nibble */
	unsigned char high_nibble = (data[1] & 0xF0);
	unsigned char low_nibble  = ((data[1] & 0x0F) << 4);

	/* high nibble to send */
	*ptr = ctrl_nibble | high_nibble;

	int err = send_i2c_data(fd, data);
	if (err == -1)
		return -1;

	err = usleep(1);  /* enable low time > 500ns*/
	if (err == -1)
		return -1;

	/* low nibble to send */
	*ptr = ctrl_nibble | low_nibble;

	err = send_i2c_data(fd, data);
	if (err == -1)
		return -1;

	err = usleep(1);  /* enable low time > 500ns*/
	if (err == -1)
		return -1;

	return 0;
}

int
send_instruction(int fd, unsigned char *data, unsigned char instruction)
{
	data[1] = instruction;

	int err = send_data(fd, data, false);
	if (err == -1)
		return -1;

	return 0;
}

int
config_lcd(int fd, unsigned char *data)
{
	/*
	 * see hitachi hd44780U datasheet at side 24
	 *
	 * send function set
	 * send display off
	 * send display clear
	 * send entry mode
	 *
	 */
	unsigned char t[] = {
		(FUNCTION_SET | TWO_LINES),
		(DISPLAY_ON_OFF_CTRL),
		(CLEAR_DISPLAY),
		(ENTRY_MODE_SET | INC_DDRAM_ADDR)
	};

	int err = 0;
	for (int i = 0; i < (int) sizeof(t); i++) {
		err = send_instruction(fd, data, t[i]);
		if (err == -1) {
			fprintf(stderr, "could not send instruction 0x%x\n", t[i]);
			continue;
		}
	}

	return 0;
}

int
enable_lcd(int fd, unsigned char *data)
{
	int err = send_instruction(fd, data,
				(DISPLAY_ON_OFF_CTRL
					| DISPLAY_ON
					| CURSOR_ON
					| CURSOR_BLINK));
	if (err == -1)
		return -1;

	return 0;
}


/*
 * ----------------------------- instructions ----------------------------------
 */


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

	if (init_lcd(fd, data) == -1) {
		fprintf(stderr, "could not init lcd\n");
		exit(EXIT_FAILURE);
	}



        /* the main loop */


	/*
	   0x80 -> erste Zeile
	   0xc0 -> zweite Zeile
	*/

	unsigned char *ptr = &data[1];

	*ptr = 0x80;
	send_data(fd, data, false);

	*ptr = 'a';
	send_data(fd, data, true);

	*ptr = 'b';
	send_data(fd, data, true);


	/* end of main loop */
	sleep(1);

	if (fd > 0)
		close(fd);

	return EXIT_SUCCESS;
}
