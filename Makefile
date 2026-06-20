all: modules-lib app-bin socket-binaries

modules-lib:
	$(MAKE) -C modules

app-bin: modules-lib
	$(MAKE) -C app

socket-binaries: modules-lib app-bin
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread app/socket_server_main.c modules/libmodules.a app/logger.o -o socket_server
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread app/socket_client_main.c modules/libmodules.a app/logger.o -o socket_client
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread app/socket_multi_server_main.c modules/libmodules.a app/logger.o -o socket_multi_server
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread app/socket_chat_main.c modules/libmodules.a app/logger.o -o socket_chat

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

test-network: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/network_test.c modules/network/network_mgr.o app/logger.c -o tests/network_test

test-package: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/package_test.c modules/package/package_mgr.o app/logger.c -o tests/package_test

test-socket: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/socket_test.c modules/socket/socket_server.o modules/socket/socket_client.o modules/socket/socket_multi_server.o app/logger.c -o tests/socket_test

test-socket-launcher: modules-lib app-bin
	gcc -Wall -Wextra -g -Iinclude -Icli -Icommon -pthread \
		tests/socket_launcher_test.c \
		app/logger.o \
		cli/parser.o cli/repl.o cli/palette.o cli/linenoise.o cli/history.o cli/autocomplete.o cli/ui.o \
		common/terminal_launcher.o \
		modules/libmodules.a -o tests/socket_launcher_test

test-kernel: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/kernel_test.c modules/kernel/kernel_mgr.o app/logger.c -o tests/kernel_test

test-shell: modules-lib
	gcc -Wall -Wextra -g -Iinclude -pthread tests/shell_test.c modules/shell/shell_mgr.o app/logger.c -o tests/shell_test

test-cli: modules-lib
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread -c cli/parser.c -o cli/parser.o
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread -c cli/palette.c -o cli/palette.o
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread -c cli/ui.c -o cli/ui.o
	gcc -Wall -Wextra -g -Iinclude -Icli -pthread tests/cli_test.c cli/parser.o cli/palette.o cli/ui.o -o tests/cli_test

clean:
	$(MAKE) -C modules clean
	$(MAKE) -C app clean
	$(MAKE) -C kernel/system_monitor clean
	rm -f tests/logger_test tests/file_test tests/process_test tests/network_test tests/package_test tests/socket_test tests/socket_launcher_test tests/kernel_test tests/shell_test tests/cli_test cli/*.o
	rm -f socket_server socket_client socket_multi_server socket_chat

.PHONY: all modules-lib app-bin socket-binaries kernel-module test-logger test-file test-process test-network test-package test-socket test-socket-launcher test-kernel test-shell test-cli clean
