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

/* common defines for driver and usage */
#include "../common.h"

#define DEV_NAME "/dev/char_driver"
#define MAX_LINE 256
#define TO_WRITE "this is a another test string with no real meaning"
#define IOCTL_TO_WRITE "set via ioctl"

static void
__attribute__((noreturn)) usage(void)
{
	fprintf(stdout, "Usage: ./usage -[rwa]     \n");
	fprintf(stdout, "       -r -> only read    \n");
	fprintf(stdout, "       -w -> only write   \n");
	fprintf(stdout, "       -i -> only ioctl   \n");
	fprintf(stdout, "       -a -> all          \n");
	putchar('\n');
	fprintf(stdout, "Examples:                 \n");
	fprintf(stdout, "       ./usage -r         \n");
	fprintf(stdout, "       ./usage -w         \n");
	fprintf(stdout, "       ./usage -a         \n");

	exit(EXIT_FAILURE);
}


/*
 * ./usage -a
 * ----------------------
 * 1. open char_driver
 * 2. read driver default string from driver
 * 3. write TO_WRITE to driver
 * 4. read new string from driver
 * 5. set IOCTL_TO_WRITE via ioctl
 * 6. read new string from driver
 */
int
main(int argc, char *argv[])
{
	if (argc != 2)
		usage();

	char buf[MAX_LINE];
	memset(buf, 0, MAX_LINE);

	/* 1. open char_driver */
	int fd = open(DEV_NAME, O_RDWR | O_EXCL);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	/* 2. read driver default string from driver */
	ssize_t n = -1;
	if ((strcmp(argv[1], "-r") == 0) || (strcmp(argv[1], "-a") == 0)) {
		fprintf(stdout, "try to read data from %s\n", DEV_NAME);

		n = read(fd, buf, MAX_LINE);
		if (n == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		fprintf(stdout, "read %d bytes \"%s\"\n", (int) n, buf);

		putchar('\n');
		sleep(1);
	}

	/* 3. write TO_WRITE to driver */
	if ((strcmp(argv[1], "-w") == 0) || (strcmp(argv[1], "-a") == 0)) {
		fprintf(stdout, "try to write \"%s\" from %s\n", TO_WRITE, DEV_NAME);

		n = write(fd, TO_WRITE, strlen(TO_WRITE));
		if (n == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		fprintf(stdout, "wrote %d bytes \"%s\"\n", (int) n, TO_WRITE);

		putchar('\n');
		sleep(1);
	}

	/* 4. read new string from driver */
	if ((strcmp(argv[1], "-w") == 0) || (strcmp(argv[1], "-a") == 0)) {
		fprintf(stdout, "try to read data from %s\n", DEV_NAME);

		memset(buf, 0, MAX_LINE);
		n = read(fd, buf, MAX_LINE);
		if (n == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		fprintf(stdout, "read %d bytes \"%s\"\n", (int) n, buf);

		putchar('\n');
		sleep(1);
	}

        /* 5. set IOCTL_TO_WRITE via ioctl */
	if ((strcmp(argv[1], "-i") == 0) || (strcmp(argv[1], "-a") == 0)) {
		int ret = ioctl(fd, IOCTL_SET_DATA, IOCTL_TO_WRITE);
		if (ret == -1) {
			perror("ioctl");
			exit(EXIT_FAILURE);
		}
	}

        /* 6. read new string from driver */
	if ((strcmp(argv[1], "-i") == 0) || (strcmp(argv[1], "-a") == 0)) {
		fprintf(stdout, "try to read data from %s\n", DEV_NAME);

		memset(buf, 0, MAX_LINE);
		n = read(fd, buf, MAX_LINE);
		if (n == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		fprintf(stdout, "read %d bytes \"%s\"\n", (int) n, buf);
	}

	close (fd);

	return EXIT_SUCCESS;
}
