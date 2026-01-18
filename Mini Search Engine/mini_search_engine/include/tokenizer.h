#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "common.h"

// Tokenizer structure for splitting text into tokens
typedef struct {
    char* text_buffer;    // Text buffer to tokenize
    char* save_ptr;       // Saved pointer for strtok_r
    const char* delimiters; // Token delimiters
} Tokenizer;

// Tokenizer operations
Tokenizer* tokenizer_create(char* text);        // Create tokenizer
char* tokenizer_next_token(Tokenizer* tokenizer); // Get next token
void tokenizer_free(Tokenizer* tokenizer);      // Free tokenizer memory

#endif