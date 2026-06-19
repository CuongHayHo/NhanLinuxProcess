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

test-system_info: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/system_info_test.c modules/systeminfo/system_info.o app/logger.c -o tests/system_info_test

test-network: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/network_test.c modules/network/network_mgr.o app/logger.c -o tests/network_test

test-package: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/package_test.c modules/package/package_mgr.o app/logger.c -o tests/package_test

test-time: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/time_test.c modules/time/time_mgr.o app/logger.c -o tests/time_test

test-scheduler: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/scheduler_test.c modules/scheduler/scheduler.o app/logger.c -o tests/scheduler_test

test-socket: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/socket_test.c modules/socket/socket_server.o app/logger.c -o tests/socket_test

clean:
	$(MAKE) -C modules clean
	$(MAKE) -C app clean
	$(MAKE) -C kernel/system_monitor clean
	rm -f tests/logger_test tests/file_test tests/process_test tests/system_info_test tests/network_test tests/package_test tests/time_test tests/scheduler_test tests/socket_test

.PHONY: all modules-lib app-bin ipc-demos kernel-module test-logger test-file test-process test-system_info test-network test-package test-time test-scheduler test-socket clean
