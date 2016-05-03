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
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


#define MAX_ADAPTER_LEN 19

static void
__attribute__((noreturn)) usage(void)
{
        fprintf(stdout, "Usage: ./usage -[1...9] -[1...9]   \n");
        fprintf(stdout, "       -1  -> adapter 1 (/dev/i2c-1)    \n");
        fprintf(stdout, "       -40 -> i2c address 0x40    \n");

        exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int fd;
	char adapter_s[MAX_ADAPTER_LEN + 1];

	if(argc != 3)
		usage();

	int adapter_nr = atoi(++(argv[1]));
	int addr = atoi(++(argv[2]));
	
	snprintf(adapter_s, MAX_ADAPTER_LEN, "/dev/i2c-%d", adapter_nr);
	printf("try to open %s@0x%d\n", adapter_s, addr);
	
	fd = open(adapter_s, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
 
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}

	__u8 reg = 0x10; /* register to access */
	char buf[10];

	if (read(fd, buf, 1) != 1)
		perror("read");
	else
		printf("read value %d", buf[0]);
	
	memset(buf, 0, sizeof(buf));
	buf[0] = reg;
	buf[1] = 0x43;
	buf[2] = 0x65;
	if (write(fd, buf, 3) != 3)
		perror("write");

	/*
	__s32 res = i2c_smbus_read_word_data(fd, reg);
	if (res < 0) {
		errno = res;
		perror("i2c_smbus_read_word_data");
	} else {
		printf("read value %ul", res);
	}
	*/
	

	close(fd);
	
	return EXIT_SUCCESS;
}
