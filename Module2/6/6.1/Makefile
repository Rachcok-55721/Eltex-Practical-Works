CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11
TARGET = phonebook.exe
LIB = libcontacts.a

all: $(TARGET)

contact_list.o: contact_list.c contact_list.h
	$(CC) $(CFLAGS) -c contact_list.c -o contact_list.o

$(LIB): contact_list.o
	$(AR) rcs $(LIB) contact_list.o

Module6.1.o: Module6.1.c contact_list.h
	$(CC) $(CFLAGS) -c Module6.1.c -o Module6.1.o

$(TARGET): Module6.1.o $(LIB)
	$(CC) Module6.1.o -L. -lcontacts -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	-del /Q Module6.1.o contact_list.o $(LIB) $(TARGET) 2>NUL

rebuild: clean all

.PHONY: all run clean rebuild
