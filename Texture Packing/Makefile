# Compiler and flags
CC      = gcc
CFLAGS  = -O2 -Wall -Iinclude
LDFLAGS = -lm

# Targets
GREEDY  = build/greedy
SA      = build/sa

# Source files
GREEDY_SRC = src/greedy.c
SA_SRC     = src/sa.c

# Default target
all: $(GREEDY) $(SA)

# Greedy algorithm
$(GREEDY): $(GREEDY_SRC)
	$(CC) $(CFLAGS) $< -o $@

# Simulated Annealing algorithm
$(SA): $(SA_SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Clean build files
clean:
	rm -f build/*

.PHONY: all clean
