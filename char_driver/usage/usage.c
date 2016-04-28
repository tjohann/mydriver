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

#define DEV_NAME "/dev/char_driver"
#define MAX_LINE 256
#define TO_WRITE "this is a test string with no meaning"


static void usage(void)
{
	fprintf(stdout, "Usage: ./usage -[rwa]     \n");
	fprintf(stdout, "       -r -> only read    \n");
	fprintf(stdout, "       -w -> only write   \n");
	fprintf(stdout, "       -a -> both         \n");
	putchar('\n');
	fprintf(stdout, "Examples:                 \n");
	fprintf(stdout, "       ./usage -r         \n");
	fprintf(stdout, "       ./usage -w         \n");
	fprintf(stdout, "       ./usage -a         \n");

	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
	if (argc != 2)
		usage();
	
	char buf[MAX_LINE];
	memset(buf, 0, MAX_LINE);

	int fd = open(DEV_NAME, O_RDWR | O_EXCL);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	ssize_t n = -1;
	
	if ((strcmp(argv[1], "-r") == 0) || (strcmp(argv[1], "-a") == 0)) {
		fprintf(stdout, "try to read data from %s\n", DEV_NAME);
		
		n = read(fd, buf, MAX_LINE);
		if (n == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		
		fprintf(stdout, "read %d bytes \"%s\"\n", (int) n, buf);
	}

	if ((strcmp(argv[1], "-w") == 0) || (strcmp(argv[1], "-a") == 0)) {
		fprintf(stdout, "try to write \"%s\" from %s\n", TO_WRITE, DEV_NAME);

		n = write(fd, buf, strlen(TO_WRITE));
		if (n == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}
		
	}	

	close (fd);
	
	return EXIT_SUCCESS;
}
