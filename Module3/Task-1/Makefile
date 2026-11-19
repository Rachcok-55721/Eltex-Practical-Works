CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGET = task1

all: $(TARGET)

$(TARGET): task1.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
