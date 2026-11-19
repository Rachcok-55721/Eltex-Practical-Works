CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGET = task4

all: $(TARGET)

$(TARGET): task4.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
