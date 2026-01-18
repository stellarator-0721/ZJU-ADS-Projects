# === Compiler Settings ===
CC = gcc
CFLAGS = -O2 -std=c11 -Wall -I$(INCDIR)
LDFLAGS = -lm

# === Directory Settings ===
SRCDIR = src
INCDIR = include
OBJDIR = build
BINDIR = bin
QUERYDIR = Queries
TARGET = dijkstra_test
QUERY_GEN = generate_queries

# === Source Files Definition ===
MAIN_SRC = $(SRCDIR)/main.c $(SRCDIR)/dijkstra.c $(SRCDIR)/graph.c $(SRCDIR)/fibheap.c $(SRCDIR)/pairingheap.c
MAIN_OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(MAIN_SRC))

# 生成查询文件需要所有相关的对象文件
QUERY_SRC = $(SRCDIR)/generate_queries.c $(SRCDIR)/graph.c $(SRCDIR)/dijkstra.c $(SRCDIR)/fibheap.c $(SRCDIR)/pairingheap.c
QUERY_OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(QUERY_SRC))

# === MinGW Windows Environment Commands ===
MKDIR = if not exist "$(1)" mkdir "$(1)"
RMDIR = rmdir /S /Q
RM = del /Q
TARGET_EXT = .exe
NULL_DEVICE = 2>nul

# === Main Targets ===
$(BINDIR)/$(TARGET)$(TARGET_EXT): $(MAIN_OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) $(LDFLAGS)

$(BINDIR)/$(QUERY_GEN)$(TARGET_EXT): $(QUERY_OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(QUERY_OBJ) $(LDFLAGS)

# === Create Necessary Directories ===
$(OBJDIR):
	$(call MKDIR,$(OBJDIR))

$(BINDIR):
	$(call MKDIR,$(BINDIR))

$(QUERYDIR):
	$(call MKDIR,$(QUERYDIR))

# === General Compilation Rules ===
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# === Query Generation Target ===
generate_queries: $(BINDIR)/$(QUERY_GEN)$(TARGET_EXT) | $(QUERYDIR)
	@echo Generating query files...
	$(BINDIR)/$(QUERY_GEN)$(TARGET_EXT) data\USA-road-d.USA.gr $(QUERYDIR)

# === Test Targets ===
test_file: $(BINDIR)/$(TARGET)$(TARGET_EXT)
	@echo Running file mode tests...
	$(BINDIR)/$(TARGET)$(TARGET_EXT) data\USA-road-d.USA.gr file $(QUERYDIR)

test_random: $(BINDIR)/$(TARGET)$(TARGET_EXT)
	@echo Running random query tests...
	$(BINDIR)/$(TARGET)$(TARGET_EXT) data\USA-road-d.USA.gr random 100

test_quick: $(BINDIR)/$(TARGET)$(TARGET_EXT)
	@echo Running quick tests...
	$(BINDIR)/$(TARGET)$(TARGET_EXT) data\USA-road-d.USA.gr quick $(QUERYDIR) small_test_queries_10.txt 5

# === Build All ===
all: $(BINDIR)/$(TARGET)$(TARGET_EXT) $(BINDIR)/$(QUERY_GEN)$(TARGET_EXT)

# === Debug Version ===
debug: CFLAGS = -g -DDEBUG -std=c11 -Wall -I$(INCDIR)
debug: all

# === Release Version ===
release: CFLAGS = -O3 -std=c11 -Wall -I$(INCDIR)
release: all

# === Clean Rules ===
clean:
	-$(RM) $(OBJDIR)\*.o $(NULL_DEVICE)
	-$(RMDIR) $(OBJDIR) $(NULL_DEVICE)
	-$(RM) $(BINDIR)\$(TARGET)$(TARGET_EXT) $(NULL_DEVICE)
	-$(RM) $(BINDIR)\$(QUERY_GEN)$(TARGET_EXT) $(NULL_DEVICE)
	-$(RMDIR) $(BINDIR) $(NULL_DEVICE)

clean_all: clean
	-$(RMDIR) $(QUERYDIR) $(NULL_DEVICE)

# === Show Help Information ===
help:
	@echo Available targets:
	@echo "  all              - Build all programs (default)"
	@echo "  generate_queries - Generate query files"
	@echo "  test_file        - Run file mode tests"
	@echo "  test_random      - Run random query tests (100 queries)"
	@echo "  test_quick       - Run quick tests with small_test_queries_10.txt"
	@echo "  debug            - Build debug version"
	@echo "  release          - Build release version"
	@echo "  clean            - Clean build files"
	@echo "  clean_all        - Clean all generated files"
	@echo "  help             - Show this help information"

# === File Dependencies ===
$(OBJDIR)/main.o: $(SRCDIR)/main.c $(INCDIR)/graph.h $(INCDIR)/dijkstra.h
$(OBJDIR)/dijkstra.o: $(SRCDIR)/dijkstra.c $(INCDIR)/dijkstra.h $(INCDIR)/graph.h $(INCDIR)/fibheap.h $(INCDIR)/pairingheap.h
$(OBJDIR)/graph.o: $(SRCDIR)/graph.c $(INCDIR)/graph.h
$(OBJDIR)/fibheap.o: $(SRCDIR)/fibheap.c $(INCDIR)/fibheap.h
$(OBJDIR)/pairingheap.o: $(SRCDIR)/pairingheap.c $(INCDIR)/pairingheap.h
$(OBJDIR)/generate_queries.o: $(SRCDIR)/generate_queries.c $(INCDIR)/generate_queries.h $(INCDIR)/graph.h $(INCDIR)/dijkstra.h

.PHONY: all generate_queries test_file test_random test_quick debug release clean clean_all help