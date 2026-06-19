all: modules-lib app-bin

modules-lib:
	$(MAKE) -C modules

app-bin: modules-lib
	$(MAKE) -C app

ipc-demos:
	$(MAKE) -C modules build-ipc

kernel-module:
	$(MAKE) -C kernel/system_monitor

test-logger: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/logger_test.c app/logger.c -o tests/logger_test

test-file: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/file_test.c modules/file/file_mgr.o app/logger.c -o tests/file_test

test-process: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/process_test.c \
		modules/process/process_mgr.o \
		modules/process/demo/fork_demo.o \
		modules/process/demo/exec_demo.o \
		modules/process/demo/wait_demo.o \
		modules/process/demo/zombie_demo.o \
		modules/process/demo/orphan_demo.o \
		modules/process/demo/daemon_demo.o \
		app/logger.c -o tests/process_test

clean:
	$(MAKE) -C modules clean
	$(MAKE) -C app clean
	$(MAKE) -C kernel/system_monitor clean
	rm -f tests/logger_test tests/file_test tests/process_test

.PHONY: all modules-lib app-bin ipc-demos kernel-module test-logger test-file test-process clean
