#
# gpio_script
#

SCRIPT1 = gpio_script.sh
SCRIPT2 = gpio_swt_led_script.sh

all:
	@echo "+-----------------------------------------+"
	@echo "| Nothing to build -> it's a shell script |"
	@echo "+-----------------------------------------+"

install: $(SCRIPT1) $(SCRIPT2)
	install $(SCRIPT1) $(HOME)/bin/$(SCRIPT1)
	install $(SCRIPT2) $(HOME)/bin/$(SCRIPT2)

uninstall:
	rm -rf $(HOME)/bin/$(SCRIPT1)
	rm -rf $(HOME)/bin/$(SCRIPT2)

clean:
	rm -rf *~
	rm -rf *.o

distclean: clean

.PHONY: all clean uninstall
