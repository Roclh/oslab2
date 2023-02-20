obj-m += kmod.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc user.c -o user
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm user
	rm *[0-9]
test:
	sudo dmesg -C
	sudo insmod kmod.ko
	sudo dmesg
close:
	sudo rmmod kmod.ko
	sudo dmesg
