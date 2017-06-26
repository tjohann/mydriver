#
# build usage.c
#

CC = gcc
CFLAGS = -Wall -Wextra

PROG = usage
OBJ = usage.o

%.o: %.c
	$(CC) -c -o $@ $<

$(PROG): $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

install: $(PROG)
	install $(PROG) $(HOME)/bin/$(PROG)_gpio_irq_lcd_hd44780

uninstall:
	rm -rf $(HOME)/bin/$(PROG)_gpio_irq_lcd_hd44780

clean:
	rm -rf *~
	rm -rf *.o
	rm -rf $(PROG)

distclean: clean

.PHONY: clean uninstall
