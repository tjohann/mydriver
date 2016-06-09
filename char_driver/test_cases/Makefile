#
# build the different testcases
#

CC = gcc
CFLAGS = -Wall -Wextra

PROG = test_cases
OBJ = test_cases.o test_case_01.o test_case_02.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROG): $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

install:
	@echo "Nothing to install"

uninstall:
	@echo "Nothing to install"

clean:
	rm -rf *~
	rm -rf .*~
	rm -rf *.o
	rm -rf $(PROG)

distclean: clean

test: $(PROG)
	(./test_cases > test_result.txt 2>&1)

.PHONY: clean uninstall install
