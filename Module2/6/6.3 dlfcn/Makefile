CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
LDLIBS = -ldl -lm
PLUGIN_SOURCES := $(wildcard plugins/*.c)
PLUGIN_LIBS := $(PLUGIN_SOURCES:.c=.so)

.PHONY: all clean run
all: Module6.3 $(PLUGIN_LIBS)

Module6.3: Module6.3.c plugin_api.h
	$(CC) $(CFLAGS) Module6.3.c -o $@ $(LDLIBS)

plugins/%.so: plugins/%.c plugin_api.h
	$(CC) $(CFLAGS) -fPIC -shared -I. $< -o $@ -lm

run: all
	./Module6.3

clean:
	rm -f Module6.3 plugins/*.so
