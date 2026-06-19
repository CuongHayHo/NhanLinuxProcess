KDIR = /lib/modules/$(shell uname -r)/build

build:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
