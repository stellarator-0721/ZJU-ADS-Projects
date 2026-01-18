#include "hash_table.h"

/**
 * DJB2 hash function - produces hash value for a given key
 * @param key The string to hash
 * @param table_size Size of the hash table
 * @return Hash value (index in the hash table)
 */
unsigned int hash(const char *key, int table_size) {
    unsigned long hash = 5381;  // DJB2 initial hash value
    int c;
    // Iterate through each character in the key
    while ((c = *key++)) {
        // hash * 33 + c (optimized as (hash << 5) + hash)
        hash = ((hash << 5) + hash) + c;
    }
    return hash % table_size;  // Ensure index is within table bounds
}

/**
 * Create and initialize a new hash table
 * @param size The size of the hash table (number of buckets)
 * @return Pointer to the newly created hash table, or NULL on failure
 */
HashTable* ht_create(int size) {
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;
    
    ht->size = size;
    // Allocate and initialize table array with NULL pointers
    ht->table = calloc(size, sizeof(HashNode*));
    if (!ht->table) {
        free(ht);
        return NULL;
    }
    return ht;
}

/**
 * Insert a key-value pair into the hash table
 * If key already exists, the function does nothing (no update)
 * @param ht The hash table to insert into
 * @param key The key string
 * @param value The value to associate with the key
 */
void ht_insert(HashTable *ht, const char *key, void *value) {
    unsigned int index = hash(key, ht->size);
    HashNode *current = ht->table[index];

    // Check if key already exists in the chain
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return;  // Key already exists, do nothing
        }
        current = current->next;
    }

    // Create new node and insert at the beginning of the chain
    HashNode *new_node = malloc(sizeof(HashNode));
    new_node->key = strdup(key);      // Duplicate key string
    new_node->value = value;          // Store value pointer
    new_node->next = ht->table[index]; // Point to current head of chain
    ht->table[index] = new_node;      // Make new node the head
}

/**
 * Search for a value by key in the hash table
 * @param ht The hash table to search in
 * @param key The key to search for
 * @return The associated value if found, NULL otherwise
 */
void* ht_search(HashTable *ht, const char *key) {
    unsigned int index = hash(key, ht->size);
    HashNode *current = ht->table[index];
    
    // Traverse the chain at the computed index
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->value;  // Key found, return value
        }
        current = current->next;
    }
    return NULL;  // Key not found
}

/**
 * Free all memory associated with the hash table
 * @param ht The hash table to free
 * @param free_value Optional function to free values (can be NULL)
 */
void ht_free(HashTable *ht, void (*free_value)(void*)) {
    if (!ht) return;
    
    // Iterate through all buckets
    for (int i = 0; i < ht->size; ++i) {
        HashNode *current = ht->table[i];
        // Free all nodes in the chain
        while (current != NULL) {
            HashNode *temp = current;
            current = current->next;
            free(temp->key);  // Free duplicated key string
            if (free_value) {
                free_value(temp->value);  // Free value if callback provided
            }
            free(temp);  // Free node itself
        }
    }
    free(ht->table);  // Free table array
    free(ht);         // Free hash table structure
}