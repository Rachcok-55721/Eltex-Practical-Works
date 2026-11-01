CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = calculator.exe
PLUGIN_SOURCES = $(wildcard plugins/*.c)
PLUGIN_DLLS = $(PLUGIN_SOURCES:.c=.dll)

all: $(TARGET) $(PLUGIN_DLLS)

$(TARGET): Module6.3.c plugin_api.h
	$(CC) $(CFLAGS) Module6.3.c -o $(TARGET) -lm

plugins/%.dll: plugins/%.c plugin_api.h
	$(CC) $(CFLAGS) -shared -o $@ $< -lm

run: all
	.\$(TARGET)

clean:
	-del /Q $(TARGET) 2>NUL
	-del /Q plugins\*.dll 2>NUL

rebuild: clean all

.PHONY: all run clean rebuild
