CC = clang

PREFIX := /usr/local
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include/coglink
DOCS_DIR = ../CoglinDocs

SRC_DIR = lib
OBJ_DIR = obj

CFLAGS := -Wall -Wextra -Wpedantic
LDFLAGS := -std=gnu99 -Iinclude -Iexternal
LFLAGS := -lcurl -ldiscord

SRCS = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))

CogLink: $(OBJS)
	ar qv libcoglink.a $(OBJS)

$(OBJ_DIR)/%.o: lib/%.c | $(OBJ_DIR)
	$(CC) -c -fPIC $< -o $@ $(LDFLAGS) $(CFLAGS)

$(OBJ_DIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf CogLink $(OBJ_DIR)

debug: CFLAGS += -g
debug: CogLink

install:
	mkdir -p $(INCLUDEDIR)
	cp include/* $(INCLUDEDIR)
	mv libcoglink.a $(LIBDIR)

gen_docs:
	doxygen Doxyfile
	rm -f $(DOCS_DIR)/* || true
	rm -rf $(DOCS_DIR)/search
	mv $(DOCS_DIR)/html/* $(DOCS_DIR)
	rm -rf $(DOCS_DIR)/html
