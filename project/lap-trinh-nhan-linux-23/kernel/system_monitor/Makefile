KDIR = /lib/modules/$(shell uname -r)/build

build:
	make -C $(KDIR) M=$(shell pwd) modules

clean:
	make -C $(KDIR) M=$(shell pwd) clean

insmod:
	sudo insmod system_monitor.ko

rmmod:
	sudo rmmod system_monitor

.PHONY: build clean insmod rmmod
