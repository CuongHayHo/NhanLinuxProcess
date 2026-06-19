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

clean:
	$(MAKE) -C modules clean
	$(MAKE) -C app clean
	$(MAKE) -C kernel/system_monitor clean
	rm -f tests/logger_test tests/file_test

.PHONY: all modules-lib app-bin ipc-demos kernel-module test-logger test-file clean
