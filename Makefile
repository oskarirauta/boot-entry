SRC_PATH?=$(PWD)
MODULES_PATH?=/lib/modules/$(shell uname -r)/build
MAKE?=$(which make)
obj-m += boot_entry.o

all:
	$(MAKE) -C $(MODULES_PATH) M=$(SRC_PATH) modules

clean:
	$(MAKE) -C $(MODULES_PATH) M=$(SRC_PATH) clean
