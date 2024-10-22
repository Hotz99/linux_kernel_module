# https://www.gnu.org/software/make/manual/make.html

EXTRA_CFLAGS += -I/usr/include

KERNEL_SRC := /lib/modules/$(shell uname -r)/build
MODULE_NAME := my_module

obj-m := $(MODULE_NAME).o

# default target: build the module
all:
	make -C $(KERNEL_SRC) M=$(PWD) modules

# clean target: clean the build directory
clean:
	rm -rf *.o *.ko *.mod *.mod.c *.symvers *.order .*.cmd
