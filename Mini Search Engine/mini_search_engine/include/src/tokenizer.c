#include "tokenizer.h"

// Default delimiters for tokenizing text: space, tab, newline, punctuation, etc.
static const char* default_delimiters = " \t\n\r\v\f,.;:?!'\"()[]{}*<>&-`~_";

/**
 * Convert a string to lowercase in place
 * @param str The string to convert (modified directly)
 */
static void to_lower_in_place(char* str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

/**
 * Create and initialize a new tokenizer
 * @param text The text string to tokenize (will be modified in place)
 * @return Pointer to newly created Tokenizer, or NULL on failure
 */
Tokenizer* tokenizer_create(char* text) {
    if (!text) return NULL;
    
    Tokenizer* tokenizer = malloc(sizeof(Tokenizer));
    if (!tokenizer) return NULL;
    
    // Initialize tokenizer fields
    tokenizer->text_buffer = text;
    tokenizer->save_ptr = NULL;
    tokenizer->delimiters = default_delimiters;
    
    // Extract and process the first token
    char* first_token = strtok_r(tokenizer->text_buffer, tokenizer->delimiters, &tokenizer->save_ptr);
    if (first_token) {
        to_lower_in_place(first_token);
        tokenizer->text_buffer = first_token; // Store first token for immediate return
    } else {
        tokenizer->text_buffer = NULL; // No tokens found
    }
    
    return tokenizer;
}

/**
 * Get the next token from the tokenizer
 * @param tokenizer The tokenizer instance
 * @return Next token as a string, or NULL if no more tokens
 */
char* tokenizer_next_token(Tokenizer* tokenizer) {
    if (!tokenizer) return NULL;

    // Return the first token that was pre-extracted during creation
    if (tokenizer->text_buffer) {
        char* current_token = tokenizer->text_buffer;
        tokenizer->text_buffer = NULL; // Mark as consumed
        return current_token;
    }

    // Get next token using strtok_r (thread-safe version)
    char* token = strtok_r(NULL, tokenizer->delimiters, &tokenizer->save_ptr);
    if (token) {
        to_lower_in_place(token); // Convert to lowercase for consistency
    }
    return token;
}

/**
 * Free the tokenizer structure (does not free the original text buffer)
 * @param tokenizer The tokenizer to free
 */
void tokenizer_free(Tokenizer* tokenizer) {
    if (tokenizer) {
        free(tokenizer);
    }
}