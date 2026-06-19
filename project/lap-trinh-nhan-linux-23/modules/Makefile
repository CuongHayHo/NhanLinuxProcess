CC = gcc
CFLAGS = -Wall -Wextra -g -I../include -pthread
AR = ar
ARFLAGS = rcs

OBJS = file/file_mgr.o \
       process/process_mgr.o \
       process/demo/fork_demo.o \
       process/demo/exec_demo.o \
       process/demo/wait_demo.o \
       process/demo/zombie_demo.o \
       process/demo/orphan_demo.o \
       process/demo/daemon_demo.o \
       signal/signal_mgr.o \
       socket/socket_server.o \
       socket/socket_client.o \
       socket/socket_mgr.o \
       socket/socket_multi_server.o \
       network/network_mgr.o \
       scheduler/scheduler.o \
       package/package_mgr.o \
       time/time_mgr.o \
       kernel/kernel_mgr.o \
       ipc/ipc_mgr.o \
       shell/shell_mgr.o \
       systeminfo/system_info.o \
       logviewer/log_viewer.o \
       core/string_util.o \
       core/time_util.o

libmodules.a: $(OBJS)
	$(AR) $(ARFLAGS) libmodules.a $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) libmodules.a
	$(MAKE) -C ipc clean

build-ipc:
	$(MAKE) -C ipc

.PHONY: clean build-ipc
