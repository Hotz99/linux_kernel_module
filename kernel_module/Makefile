EXTRA_CFLAGS += -I/usr/include

KERNEL_SRC := /lib/modules/$(shell uname -r)/build

MODULE_NAME := my_module

obj-m := $(MODULE_NAME).o

all: $(MODULE_NAME).ko

$(MODULE_NAME).ko: $(MODULE_NAME).c
	make -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean

.PHONY: all clean
