CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pthread
LDLIBS = -lrt
TARGET = task3

all: $(TARGET)

$(TARGET): task3.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
