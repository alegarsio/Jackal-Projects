# Compiler Setup
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -g
LDFLAGS = -lm

OBJDIR = obj

# --- DAFTAR FILE SOURCE (HANYA YANG DIPAKAI) ---
SRC = src/common.c \
      src/lexer.c \
      src/parser.c \
      src/env.c \
      src/value.c \
      src/eval.c \
      src/compiler/compiler.c \
      src/vm/vm.c \
      src/main.c

# (Perhatikan: src/compiler/transpiler.c SUDAH DIHAPUS dari sini)

# --- LOGIKA OTOMATIS ---
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