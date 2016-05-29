/*
 * usage.c -> show usage of gpio_irq_driver
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

#define DEV_NAME "/dev/gpio_irq_driver_simple"
#define MAX_LINE 256

static void
__attribute__((noreturn)) usage(void)
{
	fprintf(stdout, "Usage: ./usage -[h]    \n");
	exit(EXIT_FAILURE);
}

static int
open_device(void)
{
	int fd = -1;
	
	fd = open(DEV_NAME, O_RDONLY);
	if (fd == -1)
		return -1;

	return fd;
}

static int
work_mode(int fd)
{
	int value = 0;
        size_t len = sizeof(value);
        ssize_t n = 0;
	
	for (;;) {
		n = read(fd, &value, len);
		if (n == -1)
			perror("read");
		printf("%s: %d IRQ's occured\n", DEV_NAME, value);
	}
	
	return 0;  /* should never reached */
}

int
main(int argc, char *argv[])
{
	int c;
	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		case 'h':
			usage();
			break;
		default:
			fprintf(stderr, "ERROR: no valid argument\n");
			usage();
		}
	}

	int fd = open_device();
	if (fd == -1)
		usage();

	if (work_mode(fd) == -1)
		usage();

	close(fd);
	return EXIT_SUCCESS;
}
