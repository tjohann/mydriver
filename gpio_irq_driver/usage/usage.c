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

#define DEV_NAME "/dev/gpio_irq_driver"
#define DEF_PIN_READ  244

#define MAX_LINE 256

static void
__attribute__((noreturn)) usage(void)
{
	fprintf(stdout, "Usage: ./usage -[p] [PIN]                     \n");
	fprintf(stdout, "       -[p] [PIN] -> use PIN for read         \n");
	putchar('\n');
	fprintf(stdout, "Examples:                                     \n");
	fprintf(stdout, "       ./usage -p 123 (read from to pin 123)  \n");

	exit(EXIT_FAILURE);
}

static int
open_device(void)
{
	int fd = -1;

	fd = open(DEV_NAME, O_RDWR);
	if (fd == -1)
		return -1;

	return fd;
}

static int
work_mode(int fd, unsigned int pin)
{
	int value = 0;
        size_t len = sizeof(value);
        ssize_t n = 0;

	if (pin == 0)
		pin = DEF_PIN_READ;

	printf("Use pin %d\n", pin);

	n = write(fd, &pin, sizeof(unsigned int));
	if (n == -1) {
		perror("write");
		return -1;
	} else {
		printf("Wrote %d bytes\n", (int) n);
	}

	sleep(1);

	for (;;) {
		n = read(fd, &value, len);
		if (n == -1)
			perror("read");
		printf("read %d from %s\n", value, DEV_NAME);
	}

	return 0;  /* should never reached */
}

int
main(int argc, char *argv[])
{
	unsigned int pin = 0;

	int c;
	while ((c = getopt(argc, argv, "p:h")) != -1) {
		switch (c) {
		case 'p':
			pin = atoi(optarg);
			break;
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

	if (work_mode(fd, pin) == -1)
		usage();

	close(fd);
	return EXIT_SUCCESS;
}
