#
# Copyright (c) 2026 Linux System Manager Project
# All rights reserved.
#
# File: modules/process/Makefile
# Purpose: Makefile to build the Process Manager module and its lifecycle demonstrations.
#

CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include -I. -pthread

OBJS = process_mgr.o \
       demo/fork_demo.o \
       demo/exec_demo.o \
       demo/wait_demo.o \
       demo/zombie_demo.o \
       demo/orphan_demo.o \
       demo/daemon_demo.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean
