CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = phonebook.exe
DLL = contacts.dll
IMPLIB = libcontacts.dll.a

all: $(TARGET)

$(DLL): contact_list.c contact_list.h
	$(CC) $(CFLAGS) -DCONTACT_LIST_BUILD -shared -o $(DLL) contact_list.c -Wl,--out-implib,$(IMPLIB)

$(TARGET): Module6.2.c contact_list.h $(DLL)
	$(CC) $(CFLAGS) Module6.2.c -L. -lcontacts -o $(TARGET)

run: $(TARGET)
	.\$(TARGET)

clean:
	-del /Q $(TARGET) $(DLL) $(IMPLIB) 2>NUL

rebuild: clean all

.PHONY: all run clean rebuild
