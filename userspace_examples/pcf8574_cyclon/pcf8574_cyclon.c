/*
 * pcf8574_cyclon.c -> show userspace usage of i2c-dev
 *
 * GPL
 * (c) 2016-2017, thorsten.johannvorderbrueggen@t-online.de
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

#define MAX_ADAPTER_LEN 19

#define WRITE_DATA() do {					\
		if (write(fd, data, 2) != 2) {			\
			perror("write");			\
			return -1;				\
		}						\
	} while(0)

#define CLEAR_ALL() do {					\
		*ptr = 0x00;					\
		if (write(fd, data, 2) != 2) {			\
			perror("write");			\
			return -1;				\
		}						\
	} while(0)

static void
__attribute__((noreturn)) usage(void)
{
        fprintf(stdout, "Usage: ./usage [0...9] 0x[XX] [0 ... 255] \n");
        fprintf(stdout, "       1  -> adapter 1 (/dev/i2c-1)       \n");
        fprintf(stdout, "       0x20 -> i2c address 0x26           \n");
        fprintf(stdout, "       10 -> number of runs               \n");

        exit(EXIT_FAILURE);
}

static unsigned char
create_addr_byte(unsigned char addr)
{
	/* rw = 0 -> write, rw = 1 -> read */
	unsigned char rw = 0x00;

	unsigned char addr_byte = addr << 1;
	addr_byte |= rw;

	return addr_byte;
}

/*
 * for more info see
 * https://github.com/tjohann/avr_sdk/blob/master/src/libavrcyclon/libavrcyclon.c
 */
static int
cyclon_run(int fd, unsigned char count, unsigned char *data)
{
	fprintf(stdout, "will run %d times with address byte 0x%x\n",
		count, data[0]);

	unsigned char *ptr = &data[1];
	unsigned char act_count = 0;

	CLEAR_ALL();

	unsigned char i = 0;
	do {
		while (i < 7) {
			fprintf(stdout, "date: 0x%.2x\n", data[1]);

			WRITE_DATA();

			*ptr = (1 << i);
			usleep(100000);
			i++;
		}

		while (i > 0) {
			fprintf(stdout, "date: 0x%.2x\n", data[1]);

			WRITE_DATA();

			*ptr = (1 << i);
			usleep(100000);
			i--;
		}

		if (count != 0)
			act_count++;
		else
			act_count = 1;

		if (act_count == count)
			CLEAR_ALL();

	} while (act_count != count);

	return 0;
}


int
main(int argc, char *argv[])
{
	int fd = -1;
	char adapter_s[MAX_ADAPTER_LEN + 1];
	memset(adapter_s, 0, sizeof(adapter_s));

	if(argc != 4)
		usage();

	unsigned char adapter_nr = atoi(argv[1]);
	unsigned char addr = strtoul(argv[2], NULL, 0);
	unsigned char count = atoi(argv[3]);

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

	/* provide the mem (data[1]) for cyclon function */
	unsigned char data[2];
	memset(data, 0, sizeof(data));

	data[0] = create_addr_byte(addr);
	fprintf(stdout, "addr_byte = 0x%x\n", data[0]);

	/* the main loop */
	if (cyclon_run(fd, count, data) == -1)
		fprintf(stderr, "an error occured -> quit now!\n");
	else
		fprintf(stdout, "finished :-)\n");

	if (fd > 0)
		close(fd);

	return EXIT_SUCCESS;
}
