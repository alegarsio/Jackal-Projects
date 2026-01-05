UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	CC = clang
	CXX = clang++
	EXE =
	CJSON_INCLUDE = -I/opt/homebrew/include
	CJSON_LIBPATH = -L/opt/homebrew/lib
	CURL_CFLAGS = $(shell curl-config --cflags)
	CURL_LDFLAGS = $(shell curl-config --libs)
else
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	EXE = .exe
	CJSON_INCLUDE =
	CJSON_LIBPATH =
	CURL_CFLAGS =
	CURL_LDFLAGS = -lcurl
endif

CFLAGS = -Wall -Wextra -std=c11 -Iinclude -g $(CURL_CFLAGS) $(CJSON_INCLUDE)
LDFLAGS = $(CJSON_LIBPATH) $(CURL_LDFLAGS) -lcjson -lm

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
      src/vm/chunk.c \
      src/main.c

OBJ = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))

all: jackal

jackal: $(OBJ)
	$(CXX) $(OBJ) -o jackal$(EXE) $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) jackal jackal.exe

.PHONY: all clean
