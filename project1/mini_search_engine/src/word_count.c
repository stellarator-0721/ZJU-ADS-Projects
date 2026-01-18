#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "stopwords_generator.h"
#include "tokenizer.h"

#define MAX_WORD_LENGTH 100
#define HASH_SIZE 100000

// Structure for storing word frequency information in hash table
typedef struct WordNode {
    char word[MAX_WORD_LENGTH];  // The word string
    int frequency;               // Frequency count
    struct WordNode* next;       // Next node in linked list (for collision resolution)
} WordNode;

// Global variables for word counting
static WordNode* word_table_raw[HASH_SIZE];  // Hash table for word frequencies
static int total_words_raw = 0;              // Total number of words processed
static int unique_words_raw = 0;             // Number of unique words

/**
 * Hash function for string keys (DJB2 algorithm)
 * @param str The string to hash
 * @return Hash value within hash table size
 */
static unsigned int hash_raw(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash % HASH_SIZE;
}

/**
 * Read entire file content into a string
 * @param filename Path to the file
 * @return Dynamically allocated string with file content, or NULL on error
 */
static char* read_file_raw(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (length <= 0) {
        fclose(file);
        return NULL;
    }
    
    // Allocate buffer and read file
    char* buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, file);
        buffer[length] = '\0';  // Null-terminate the string
    }
    
    fclose(file);
    return buffer;
}

/**
 * Add a word to the frequency hash table or increment its count
 * @param word The word to add/count
 */
static void add_word_raw(const char* word) {
    // Skip words that are too short or too long
    if (strlen(word) <= 1 || strlen(word) >= MAX_WORD_LENGTH) return;
    
    unsigned int index = hash_raw(word);
    WordNode* current = word_table_raw[index];
    
    // Check if word already exists in hash chain
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            current->frequency++;  // Increment existing word count
            return;
        }
        current = current->next;
    }

    // Create new node for new word
    WordNode* new_node = malloc(sizeof(WordNode));
    if (!new_node) return;
    
    strcpy(new_node->word, word);
    new_node->frequency = 1;
    new_node->next = word_table_raw[index];  // Insert at head of chain
    word_table_raw[index] = new_node;
    unique_words_raw++;  // Increment unique word count
}

/**
 * Tokenize text and count word frequencies
 * @param text The text to tokenize and count
 */
static void tokenize_and_count(char* text) {
    Tokenizer* tokenizer = tokenizer_create(text);
    char* token;
    
    // Process each token
    while ((token = tokenizer_next_token(tokenizer)) != NULL) {
        add_word_raw(token);
        total_words_raw++;  // Increment total word count
    }
    
    tokenizer_free(tokenizer);
}

/**
 * Process a single file: read content and count words
 * @param filename Path to the file to process
 */
static void process_file_raw(const char* filename) {
    char* content = read_file_raw(filename);
    if (content) {
        tokenize_and_count(content);
        free(content);
    }
}

/**
 * Recursively process all files in a directory
 * @param dir_path Path to the directory to process
 */
static void process_directory_raw(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files and directories
        if (entry->d_name[0] == '.') continue;
        
        // Construct full file path
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
        
        // Check if it's a regular file
        struct stat path_stat;
        if (stat(filepath, &path_stat) != 0) continue;
        
        if (S_ISREG(path_stat.st_mode)) {
            process_file_raw(filepath);
        }
    }
    
    closedir(dir);
}

/**
 * Comparison function for sorting words by frequency (descending)
 * @param a Pointer to first WordNode pointer
 * @param b Pointer to second WordNode pointer
 * @return Negative if b should come before a, positive if a should come before b
 */
static int compare_words_raw(const void* a, const void* b) {
    const WordNode* node_a = *(const WordNode**)a;
    const WordNode* node_b = *(const WordNode**)b;
    return node_b->frequency - node_a->frequency;  // Descending order
}

/**
 * Save frequency analysis results to file
 * @param filename Output file name
 * @param all_words Array of WordNode pointers to save
 */
static void save_results_to_file(const char* filename, WordNode** all_words) {
    FILE* file = fopen(filename, "w");
    if (!file) return;
    
    // Write header and summary
    fprintf(file, "Raw Word Frequency Analysis\n");
    fprintf(file, "================================\n");
    fprintf(file, "Total words: %d\n", total_words_raw);
    fprintf(file, "Unique words: %d\n", unique_words_raw);
    fprintf(file, "\nTop %d Most Frequent Words:\n", unique_words_raw < 200 ? unique_words_raw : 200);
    fprintf(file, "%-6s %-20s %-10s %s\n", "Rank", "Word", "Frequency", "Percentage");
    fprintf(file, "------------------------------------------------\n");
    
    // Write word frequency data
    int top_n = unique_words_raw < 200 ? unique_words_raw : 200;
    for (int i = 0; i < top_n; i++) {
        double percentage = (double)all_words[i]->frequency / total_words_raw * 100;
        fprintf(file, "%-6d %-20s %-10d %.4f%%\n", 
                i + 1, all_words[i]->word, all_words[i]->frequency, percentage);
    }
    
    fclose(file);
    printf("Raw frequency results saved to: %s\n", filename);
}

/**
 * Clean up hash table and free all allocated memory
 */
static void cleanup_raw() {
    for (int i = 0; i < HASH_SIZE; i++) {
        WordNode* current = word_table_raw[i];
        while (current != NULL) {
            WordNode* temp = current;
            current = current->next;
            free(temp);
        }
        word_table_raw[i] = NULL;
    }
}

/**
 * Main function for raw word frequency analysis
 * Processes all files in directory, counts word frequencies, and saves results
 * @param data_dir Directory containing text files to analyze
 * @param freq_file_out Output file for frequency results
 */
void run_raw_word_count(const char* data_dir, const char* freq_file_out) {
    printf("--- Task 1: Raw Word Frequency Analysis ---\n");
    
    // Initialize global variables
    memset(word_table_raw, 0, sizeof(word_table_raw));
    total_words_raw = 0;
    unique_words_raw = 0;

    // Process all files in directory
    process_directory_raw(data_dir);

    // Check if any words were found
    if (total_words_raw == 0) {
        printf("No words found in directory: %s\n", data_dir);
        return;
    }

    // Create array of all words for sorting
    WordNode** all_words = malloc(unique_words_raw * sizeof(WordNode*));
    if (!all_words) return;

    // Copy words from hash table to array
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        WordNode* current = word_table_raw[i];
        while (current != NULL) {
            all_words[count++] = current;
            current = current->next;
        }
    }

    // Sort words by frequency (descending)
    qsort(all_words, unique_words_raw, sizeof(WordNode*), compare_words_raw);

    // Display results to console
    printf("%-6s %-20s %-10s %s\n", "Rank", "Word", "Frequency", "Percentage");
    printf("------------------------------------------------\n");
    int top_n = unique_words_raw < 200 ? unique_words_raw : 200;
    for (int i = 0; i < top_n; i++) {
        double percentage = (double)all_words[i]->frequency / total_words_raw * 100;
        printf("%-6d %-20s %-10d %.4f%%\n", 
               i + 1, all_words[i]->word, all_words[i]->frequency, percentage);
    }
    
    // Save results to file
    save_results_to_file(freq_file_out, all_words);
    
    // Clean up memory
    free(all_words);
    cleanup_raw();
    printf("--- Raw Word Frequency Analysis Finished ---\n\n");
}