#
# gpio_script
#

PROG = gpio_script.sh

all:
	@echo "+-----------------------------------------+"
	@echo "| Nothing to build -> it's a shell script |"
	@echo "+-----------------------------------------+"

install: $(PROG)
	install $(PROG) $(HOME)/bin/$(PROG)

uninstall:
	rm -rf $(HOME)/bin/$(PROG)

clean:
	rm -rf *~
	rm -rf *.o

distclean: clean

.PHONY: all clean uninstall
