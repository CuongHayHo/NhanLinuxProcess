KDIR = /lib/modules/`uname -r`/build

build:
	make -C $(KDIR) M=`pwd` modules

clean:
	make -C $(KDIR) M=`pwd` clean