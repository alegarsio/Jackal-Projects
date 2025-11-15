
CXX = g++

CC = gcc

CXXFLAGS = -Wall -Wextra -std=c++11 -Iinclude

CFLAGS = -Wall -Wextra -std=c11 -Iinclude

OBJDIR = obj

SRC = src/common.c src/lexer.c src/parser.c src/env.c src/value.c \
      src/eval.c \
      src/main.cpp

OBJ = $(patsubst src/%.c, $(OBJDIR)/%.o, $(filter %.c, $(SRC))) \
      $(patsubst src/%.cpp, $(OBJDIR)/%.o, $(filter %.cpp, $(SRC)))

all: jackal

jackal: $(OBJ)
	$(CXX) $(OBJ) -o jackal

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) jackal

install: jackal
	cp ./jackal /usr/local/bin/jackal

.PHONY: all clean install