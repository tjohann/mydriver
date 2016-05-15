/*
 * usage.c -> show usage of char_driver
 *
 * GPL
 * (c) 2016, thorsten.johannvorderbrueggen@t-online.de
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


#define DEV_NAME "/dev/gpio_driver"
#define MAX_LINE 256

static void
__attribute__((noreturn)) usage(void)
{
	fprintf(stdout, "Usage: ./usage -[rwa]     \n");
	fprintf(stdout, "       -r -> read PIN     \n");
	fprintf(stdout, "       -w -> toogle PIN   \n");
	putchar('\n');
	fprintf(stdout, "Examples:                 \n");
	fprintf(stdout, "       ./usage -r         \n");
	fprintf(stdout, "       ./usage -w         \n");

	exit(EXIT_FAILURE);
}

static int
open_device(char *mode)
{
	int fd = -1;

	switch (*mode) {
	case 'w':
		fd = open(DEV_NAME, O_WRONLY);
		if (fd == -1) {
			perror("open");
			return -1;
		}
		break;
	case 'r':
		fd = open(DEV_NAME, O_RDONLY);
		if (fd == -1) {
			perror("open");
			return -1;
		}
		break;
	default:
		fprintf(stderr, "mode not supported\n");
		usage();
	}

	return fd;
}


static void
work_mode(int fd, char *mode)
{
	struct timespec t;

	memset(&t, 0, sizeof(struct timespec));

	/* sleep time 0.5 sec */
	t.tv_sec = 0;
	t.tv_nsec = 500000000;

	int value = 0;
	size_t len = sizeof(value);
	ssize_t n = 0;

	switch (*mode) {
	case 'w':
		/* toogle pin */
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
		break;
	case 'r':
		/* read pin*/
		for (;;) {
			n = read(fd, &value, len);
			if (n == -1)
				perror("read");
			printf("read %d from %s\n", value, DEV_NAME);
			clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);

			value = 1;
			n = read(fd, &value, len);
			if (n == -1)
				perror("read");
			printf("read %d from %s\n", value, DEV_NAME);
			clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);
			value = 0;
		}
		break;
	default:
		fprintf(stderr, "mode not supported\n");
		usage();
	}
}

int
main(int argc, char *argv[])
{
	int fd;

	if (argc != 2)
		usage();

	char *mode = ++argv[1];

	fd = open_device(mode);
	if (fd == -1)
		usage();

	work_mode(fd, mode);

	close(fd);
	return EXIT_SUCCESS;
}
