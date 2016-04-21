#
# build example.c
#

CC = gcc
CFLAGS = -Wall -Wextra

OBJ = usage.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

usage: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean::
	rm -rf *~
	rm -rf usage

