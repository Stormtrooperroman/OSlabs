obj-m += swap_size.o
PWD := $(CURDIR)
all:
	make -C\
	/lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C\
	/lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	sudo dmesg -C
	sudo insmod swap_size.ko
	cat /proc/tsulab
	sudo rmmod swap_size.ko
	sudo dmesg