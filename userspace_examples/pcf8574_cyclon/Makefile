#
# build pcf8574_cyclon.c
#

CC = gcc
CFLAGS = -Wall -Wextra

PROG = pcf8574_cyclon
OBJ = pcf8574_cyclon.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROG): $(OBJ)
	$(CC) -o $@ $^ -lc

install: $(PROG)
	install $(PROG) $(HOME)/bin/$(PROG)

uninstall:
	rm -rf $(HOME)/bin/$(PROG)

clean:
	rm -rf *~ .*~
	rm -rf *.o
	rm -rf $(PROG)

distclean: clean

.PHONY: clean uninstall
