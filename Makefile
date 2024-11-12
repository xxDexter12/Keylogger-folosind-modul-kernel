obj-m := init.o

# Directorul curent de lucru
COMPILE_DIR := $(shell pwd)

# Calea către kernel headers pentru versiunea curentă a kernelului
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(COMPILE_DIR) modules

clean:
	rm -f -v *.o *.ko *.mod. .*.cmd *.symvers *.order