CC ?= clang

PREFIX := /usr/local
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include/coglink

CFLAGS := -Wall -Wextra -Wpedantic -Ofast -Wno-incompatible-pointer-types
LDFLAGS := -std=gnu99 -ldiscord -lcurl -pthread

all:
	@ sudo mkdir -p $(INCLUDEDIR)
	sudo cp include/* $(INCLUDEDIR)
	$(CC) -c $(CFLAGS) $(LDFLAGS) -fpic lib/*.c
	ar qv libcoglink.a *.o
	rm -rf *.o

install:
	sudo mv libcoglink.a $(LIBDIR)