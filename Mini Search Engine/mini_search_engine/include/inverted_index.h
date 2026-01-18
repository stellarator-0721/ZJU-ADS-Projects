#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include "hash_table.h"
#include "file_utils.h"

// Document node structure, represents a document containing a specific word
typedef struct DocNode {
    int doc_id;           // Document ID
    int frequency;        // Word frequency in the document
    struct DocNode *next; // Next document node
} DocNode;

// Inverted index type definition, essentially a hash table
typedef HashTable InvertedIndex;

// Inverted index operations
HashTable* load_stopwords(const char *filepath);  // Load stop words from file
InvertedIndex* build_inverted_index(const char *dirpath, HashTable *stopwords, FileList *filelist);  // Build inverted index
void free_inverted_index(InvertedIndex *index);   // Free inverted index memory

#endif