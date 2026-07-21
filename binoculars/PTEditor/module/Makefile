obj-m += pteditor.o
ccflags-y += -Wno-unused-result
INC_DIRS = -I/usr/include/linux/

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

