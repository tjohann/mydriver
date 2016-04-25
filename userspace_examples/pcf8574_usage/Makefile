#
# build pcf8574.c
#

CC = gcc
CFLAGS = -Wall -Wextra

OBJ = pcf8574.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

pcf8574: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean::
	rm -rf *.o
	rm -rf *~
	rm -rf pcf8574

