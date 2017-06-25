/*
 * usage.c -> show usage of i2c_gpio_driver_simple driver
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <time.h>

/* common defines for driver and usage */
#include "../common.h"

#define MAX_LINE 256

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
	fprintf(stdout, "Usage: ./usage -b adapter -a addr \n");
	fprintf(stdout, "       -b -> Adapter number       \n");
	fprintf(stdout, "       -a -> Address (hex)        \n");
	putchar('\n');
	fprintf(stdout, "Examples:                         \n");
	fprintf(stdout, "       ./usage -a 0x26 -b 1       \n");
	exit(EXIT_FAILURE);
}

static void
print_client_config(struct client_config *client)
{
	printf("Adapternumber: %d\n", client->adapter_nr);
	printf("Address:       0x%x\n", client->addr);
}

static bool
is_client_config_valid(struct client_config *client)
{
	if (client->adapter_nr > 20)
		printf("adapter number > %d makes no sense\n",
		       client->adapter_nr);

	if (client->addr > 128)
		printf("adapter number > %d makes no sense\n",
		       client->addr);
	return true;
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

static int
open_device(void)
{
	int fd = -1;

	fd = open(DEV_NAME, O_WRONLY);
	if (fd == -1)
		goto error;

	return fd;

error:
	fprintf(stderr, "could not open %s\n", DEV_NAME);
	return -1;
}

int
main(int argc, char *argv[])
{
	struct client_config client;
	memset(&client, 0, sizeof(client));

	int c;
	while ((c = getopt(argc, argv, "b:a:h")) != -1) {
		switch (c) {
		case 'b':
			client.adapter_nr = atoi(optarg);
			break;
		case 'a':
			client.addr = strtoul(optarg, NULL, 16);
			break;
		case 'h':
			usage();
			break;
		default:
			fprintf(stderr, "ERROR: no valid argument\n");
			usage();
		}
	}

	print_client_config(&client);

	if (!is_client_config_valid(&client)) {
		fprintf(stderr,"client config is not valid\n");
		usage();
	}

	int fd = open_device();
	if (fd == -1)
		usage();

	int ret = ioctl(fd, IOCTL_SET_I2C_CLIENT , &client);
	if (ret == -1) {
		fprintf(stderr, "could not set client values\n");
		return EXIT_FAILURE;
	}

	if (work_mode(fd) == -1)
		usage();

	close(fd);
	return EXIT_SUCCESS;
}
