/*
 * usage.c -> show usage of char_driver
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


static void
__attribute__((noreturn)) usage(void)
{
	fprintf(stdout, "Usage: ./usage -b adapter -a addr \n");
	fprintf(stdout, "       -b -> Adapter number       \n");
	fprintf(stdout, "       -a -> Address (hex)        \n");
	putchar('\n');
	fprintf(stdout, "Examples:                         \n");
	fprintf(stdout, "       ./usage -a 0x26 -b 0       \n");
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

static int
work_mode(int fd)
{
	struct timespec t;

	memset(&t, 0, sizeof(struct timespec));

	/* sleep time 0.5 sec */
	t.tv_sec = 0;
	t.tv_nsec = 500000000;

	int value = 0;
	size_t len = sizeof(value);
	ssize_t n = 0;

	/* toogle port bits */
	for (;;) {
		n = write(fd, &value, len);
		if (n == -1)
			perror("write");
		clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);

		value = 1;
		n = write(fd, &value, len);
		if (n == -1)
			perror("write");
		clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);
		value = 0;
	}

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
