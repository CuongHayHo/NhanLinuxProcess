CC = gcc
CFLAGS = -Wall -Wextra -g -I../include -pthread
LDFLAGS = -L../modules -lmodules

OBJS = main.o menu.o logger.o
TARGET = ../sysmgr

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
