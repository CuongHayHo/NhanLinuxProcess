# Makefile for the kernel modules
# Supports make, load (insmod), and unload (rmmod) operations.

all:
	$(MAKE) -C system_monitor build

clean:
	$(MAKE) -C system_monitor clean

insmod:
	sudo insmod system_monitor/system_monitor.ko

rmmod:
	sudo rmmod system_monitor

.PHONY: all clean insmod rmmod
