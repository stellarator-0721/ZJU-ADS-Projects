#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "common.h"

// Hash table node structure
typedef struct HashNode {
    char *key;           // Key (string)
    void *value;         // Value (generic pointer)
    struct HashNode *next; // Next node (for collision resolution)
} HashNode;

// Hash table structure
typedef struct HashTable {
    int size;           // Hash table size
    HashNode **table;   // Array of hash buckets
} HashTable;

// Hash table operations
unsigned int hash(const char *key, int table_size);           // Hash function
HashTable* ht_create(int size);                               // Create hash table
void ht_insert(HashTable *ht, const char *key, void *value);  // Insert key-value pair
void* ht_search(HashTable *ht, const char *key);              // Search for value by key
void ht_free(HashTable *ht, void (*free_value)(void*));       // Free hash table memory

#endif