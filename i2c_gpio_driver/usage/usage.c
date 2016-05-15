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


#define MAX_LINE 256

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


int
main(int argc, char *argv[])
{
	if (argc != 2)
		usage();

	char buf[MAX_LINE];
	memset(buf, 0, MAX_LINE);






	return EXIT_SUCCESS;
}
