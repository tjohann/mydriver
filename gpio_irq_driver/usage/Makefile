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
	gcc -o $@ $^ $(CFLAGS)

install: $(PROG)
	install $(PROG) $(HOME)/bin/$(PROG)_gpio_irq_driver

uninstall:
	rm -rf $(HOME)/bin/$(PROG)_gpio_irq_driver

clean:
	rm -rf *~
	rm -rf *.o
	rm -rf $(PROG)

distclean: clean

.PHONY: clean uninstall
