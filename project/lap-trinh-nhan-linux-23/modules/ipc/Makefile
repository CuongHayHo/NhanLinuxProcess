CC = gcc
CFLAGS = -Wall -Wextra -g -I../../include -pthread
BINDIR = bin

TARGETS = $(BINDIR)/shm_demo $(BINDIR)/sem_demo $(BINDIR)/fifo_demo $(BINDIR)/msgq_demo

all: $(BINDIR) $(TARGETS)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/shm_demo: shm_demo.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/sem_demo: sem_demo.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/fifo_demo: fifo_demo.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/msgq_demo: msgq_demo.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BINDIR)

.PHONY: all clean
