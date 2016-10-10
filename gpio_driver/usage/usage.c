/*
 * usage.c -> show usage of gpio_driver
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

/*
   #define IOCTL_SET_WRITE_PIN 0x0001  <-> MODE_WRITE
   #define IOCTL_SET_READ_PIN  0x0002  <-> MODE_READ
 */
#define MODE_WRITE 0x0001
#define MODE_READ  0x0002

static void
__attribute__((noreturn)) usage(void)
{
	fprintf(stdout, "Usage: ./usage -[rwi] [PIN]                   \n");
	fprintf(stdout, "       -r -> read PIN                         \n");
	fprintf(stdout, "       -w -> toogle PIN                       \n");
	fprintf(stdout, "       -[rw] - [PIN] -> use PIN for write    \n");
	putchar('\n');
	fprintf(stdout, "Examples:                                     \n");
	fprintf(stdout, "       ./usage -r (read from default pin)     \n");
	fprintf(stdout, "       ./usage -w (write to default pin)      \n");
	fprintf(stdout, "       ./usage -w -p 123 (write to pin 123)   \n");
	fprintf(stdout, "       ./usage -rp 321 (read from pin 321)    \n");

	exit(EXIT_FAILURE);
}

static int
open_device(unsigned char mode)
{
	int fd = -1;

	if (mode & MODE_READ) {
		fd = open(DEV_NAME, O_RDONLY);
		if (fd == -1)
			goto error;

		return fd;
	}

	if (mode & MODE_WRITE) {
		fd = open(DEV_NAME, O_WRONLY);
		if (fd == -1)
			goto error;

		return fd;
	}

error:
	fprintf(stderr, "could not open %s\n", DEV_NAME);
	return -1;
}


static int
work_mode(int fd, unsigned char mode, int pin)
{
	struct timespec t;
	memset(&t, 0, sizeof(struct timespec));

	/* sleep time 0.5 sec */
	t.tv_sec = 0;
	t.tv_nsec = 500000000;

	int value = 0;
	size_t len = sizeof(value);
	ssize_t n = 0;

	printf("In %s with mode %d and pin=%d\n", __FUNCTION__, mode, pin);

	if (pin <= 0) {
		printf("a value below <=0 makes no sense\n");
	} else {
		int ret = ioctl(fd, mode, &pin);
		if (ret == -1)
			goto error;
	}

	if (mode == MODE_WRITE) {
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

		return 0; /* should never reached */
	}

	if (mode == MODE_READ) {
		/* read pin*/
		for (;;) {
			n = read(fd, &value, len);
			if (n == -1)
				perror("read");
			printf("read %d from %s\n", value, DEV_NAME);
			clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);
		}

		return 0;  /* should never reached */
	}

error:
	return -1;
}

int
main(int argc, char *argv[])
{
	unsigned char mode = 0;
	int pin = -1;

	int c;
	while ((c = getopt(argc, argv, "rwp:h")) != -1) {
		switch (c) {
		case 'r':
			mode |= MODE_READ;
			break;
		case 'w':
			mode |= MODE_WRITE;
			break;
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

	printf("Used mode %d and pin %d\n", mode, pin);

	int fd = open_device(mode);
	if (fd == -1)
		usage();

	if (work_mode(fd, mode, pin) == -1)
		usage();

	close(fd);
	return EXIT_SUCCESS;
}
