CC = gcc
CFLAGS = -Wall -Wextra -g -I../include -I../cli -I../common -pthread
LDFLAGS = -L../modules -lmodules

OBJS = main.o menu.o logger.o \
       ../cli/repl.o \
       ../cli/linenoise.o \
       ../cli/parser.o \
       ../cli/palette.o \
       ../cli/history.o \
       ../cli/autocomplete.o \
       ../cli/ui.o \
       ../common/terminal_launcher.o

TARGET = ../sysmgr

all: $(TARGET)

$(TARGET): $(OBJS) ../modules/libmodules.a
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

../cli/%.o: ../cli/%.c
	$(CC) $(CFLAGS) -c $< -o $@

../common/%.o: ../common/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f ../cli/*.o ../common/*.o

.PHONY: all clean
