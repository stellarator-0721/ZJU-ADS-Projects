CC = gcc
CFLAGS = -Iinclude -Ilib -Wall -g
LDFLAGS =

SRCDIR = src
OBJDIR = obj
BINDIR = .
LIBDIR = lib

ENGINE_TARGET = $(BINDIR)/mini_search_engine
ENGINE_SRCS = $(SRCDIR)/main.c \
              $(SRCDIR)/file_utils.c \
              $(SRCDIR)/hash_table.c \
              $(SRCDIR)/inverted_index.c \
              $(SRCDIR)/query.c \
              $(SRCDIR)/tokenizer.c \
              $(LIBDIR)/stmr.c
ENGINE_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(filter $(SRCDIR)/%.c, $(ENGINE_SRCS))) \
              $(patsubst $(LIBDIR)/%.c, $(OBJDIR)/%.o, $(filter $(LIBDIR)/%.c, $(ENGINE_SRCS)))

STOPWORDS_TARGET = $(BINDIR)/get_stopwords
STOPWORDS_SRCS = $(SRCDIR)/get_stopwords.c \
                 $(SRCDIR)/stopword_analyzer.c \
                 $(SRCDIR)/word_count.c \
                 $(SRCDIR)/hash_table.c \
                 $(SRCDIR)/tokenizer.c
STOPWORDS_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(STOPWORDS_SRCS))

all: $(ENGINE_TARGET) $(STOPWORDS_TARGET)
	@echo "All targets built successfully."
	@echo "Executables: ./mini_search_engine, ./get_stopwords"

$(ENGINE_TARGET): $(ENGINE_OBJS)
	$(CC) $(ENGINE_OBJS) -o $@ $(LDFLAGS)
	@echo "Built $(ENGINE_TARGET)"

$(STOPWORDS_TARGET): $(STOPWORDS_OBJS)
	$(CC) $(STOPWORDS_OBJS) -o $@ $(LDFLAGS)
	@echo "Built $(STOPWORDS_TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@-mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(LIBDIR)/%.c
	@-mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up object and binary files..."
	-rm -rf $(OBJDIR)
	-rm -f $(ENGINE_TARGET) $(STOPWORDS_TARGET)
	-rm -f $(ENGINE_TARGET).exe $(STOPWORDS_TARGET).exe
	@echo "Cleanup complete."

.PHONY: all clean