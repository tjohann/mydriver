/*
 * test_cases.c -> testcases for char_driver.c
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

#include "test_cases.h"

static void
__attribute__((noreturn)) usage(void)
{
	fprintf(stdout, "Usage: ./test_cases        \n");
	putchar('\n');

	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	bool ret = test_cases_01();
	fprintf(stderr, "test_cases_01 result: %s\n", ret ? "success" : "failed" );

	ret = test_cases_02();
	fprintf(stderr, "test_cases_02 result: %s\n", ret ? "success" : "failed" );

	return EXIT_SUCCESS;
}
