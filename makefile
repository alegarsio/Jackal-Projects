CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

OBJDIR = obj

# --- PERBAIKI BARIS INI ---
SRC = src/common.c src/lexer.c src/parser.c src/env.c src/value.c src/eval.c src/main.c
# --- BATAS PERBAIKAN ---

OBJ = $(patsubst src/%.c, $(OBJDIR)/%.o, $(SRC))

jackal: $(OBJ)
	$(CC) $(OBJ) -o jackal

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJ): | $(OBJDIR)

clean:
	rm -rf $(OBJDIR) jackal