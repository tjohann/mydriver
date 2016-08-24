#
# build usage.c
#

CC = gcc
CFLAGS = -Wall -Wextra

PROG = usage
OBJ = usage.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROG): $(OBJ)
	gcc -o $@ $^

install: $(PROG)
	install $(PROG) $(HOME)/bin/$(PROG)_i2c_gpio_driver_simple

uninstall:
	rm -rf $(HOME)/bin/$(PROG)_i2c_gpio_driver_simple

clean:
	rm -rf *~
	rm -rf *.o
	rm -rf $(PROG)

distclean: clean

.PHONY: clean uninstall
