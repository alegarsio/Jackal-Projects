
CC = gcc
CXX = g++

CURL_CFLAGS = $(shell curl-config --cflags)
CURL_LDFLAGS = $(shell curl-config --libs)

CJSON_INCLUDE_PATH = -I/opt/homebrew/include
CJSON_LIBRARY = -lcjson
CJSON_LIBRARY_PATH = -L/opt/homebrew/lib 

CFLAGS = -Wall -Wextra -std=c11 -Iinclude -g $(CURL_CFLAGS) $(CJSON_INCLUDE_PATH)

LDFLAGS = $(CJSON_LIBRARY_PATH) $(CURL_LDFLAGS) $(CJSON_LIBRARY) -lm

OBJDIR = obj

SRC = src/common.c \
      src/lexer.c \
      src/parser.c \
      src/env.c \
      src/value.c \
      src/eval.c \
      src/vm/debug.c \
      src/compiler/compiler.c \
      src/vm/vm.c \
      src/main.c

OBJ = $(patsubst src/%.c, $(OBJDIR)/%.o, $(SRC))

all: jackal

jackal: $(OBJ)
	$(CXX) $(OBJ) -o jackal $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) jackal

.PHONY: all clean