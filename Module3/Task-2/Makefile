CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGET = task2

all: $(TARGET)

$(TARGET): task2.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
