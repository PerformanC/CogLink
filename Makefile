CC ?= clang

PREFIX := /usr/local
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include/coglink

CFLAGS := -Wall -Wextra -Wpedantic -Wno-incompatible-pointer-types
LDFLAGS := -std=gnu99 -I$(PREFIX)/include

all:
	@ mkdir -p $(INCLUDEDIR)
	cp include/* $(INCLUDEDIR)
	$(CC) -c $(CFLAGS) $(LDFLAGS) -Ofast -fpic lib/*.c
	ar qv libcoglink.a *.o
	rm -rf *.o

install:
	mv libcoglink.a $(LIBDIR)