#include "inverted_index.h"
#include "../lib/stmr.h"
#include "tokenizer.h"

/**
 * Free a posting list (linked list of DocNodes)
 * @param list Pointer to the head of the posting list
 */
void free_posting_list(void *list) {
    DocNode *current = (DocNode*)list;
    while (current) {
        DocNode *temp = current;
        current = current->next;
        free(temp);
    }
}

/**
 * Free the entire inverted index and all its posting lists
 * @param index The inverted index to free
 */
void free_inverted_index(InvertedIndex *index) {
    ht_free(index, free_posting_list);
}

/**
 * Load stop words from a file into a hash table
 * @param filepath Path to the stop words file
 * @return Hash table containing stop words, or NULL on error
 */
HashTable* load_stopwords(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Could not open stopwords file");
        return NULL;
    }
    
    HashTable *stopwords_ht = ht_create(1000);
    char word[MAX_WORD_LEN];
    
    // Read each word from the file and insert into hash table
    while (fscanf(file, "%99s", word) != EOF) {
        ht_insert(stopwords_ht, word, (void*)1);
    }
    
    fclose(file);
    return stopwords_ht;
}

/**
 * Process a token: check against stop words, stem, and update inverted index
 * @param token The token to process
 * @param doc_id The current document ID
 * @param index The inverted index to update
 * @param stopwords Hash table of stop words to filter out
 */
void process_token(char *token, int doc_id, InvertedIndex *index, HashTable *stopwords) {
    // Skip if token is a stop word
    if (ht_search(stopwords, token)) {
        return;
    }

    // Stem the token using Porter stemmer
    int end = stem(token, 0, strlen(token) - 1);
    token[end + 1] = '\0';
    
    // Skip empty tokens after stemming
    if (strlen(token) == 0) return;

    // Get existing posting list for this token
    DocNode *posting_list = ht_search(index, token);
    
    if (posting_list == NULL) {
        // First occurrence of this token - create new posting list
        DocNode *new_doc = malloc(sizeof(DocNode));
        new_doc->doc_id = doc_id;
        new_doc->frequency = 1;
        new_doc->next = NULL;
        ht_insert(index, token, new_doc);
    } else {
        // Token exists - find if current document is already in posting list
        DocNode *current = posting_list;
        DocNode *last = NULL;
        
        while (current) {
            if (current->doc_id == doc_id) {
                // Document already exists - increment frequency
                current->frequency++;
                return;
            }
            last = current;
            current = current->next;
        }
        
        // Document not in posting list - add new document node
        DocNode *new_doc = malloc(sizeof(DocNode));
        new_doc->doc_id = doc_id;
        new_doc->frequency = 1;
        new_doc->next = NULL;
        last->next = new_doc;
    }
}

/**
 * Build inverted index from all files in a directory
 * @param dirpath Directory containing the documents to index
 * @param stopwords Hash table of stop words to exclude
 * @param filelist Structure to store file information and document lengths
 * @return The built inverted index, or NULL on error
 */
InvertedIndex* build_inverted_index(const char *dirpath, HashTable *stopwords, FileList *filelist) {
    InvertedIndex *index = ht_create(HASH_TABLE_SIZE);
    
    // Allocate memory for document lengths
    filelist->doc_lengths = calloc(filelist->count, sizeof(int));
    if (!filelist->doc_lengths) {
        perror("Failed to allocate memory for doc_lengths");
        return NULL;
    }

    // Process each document
    for (int i = 0; i < filelist->count; ++i) {
        printf("Indexing document %d/%d: %s\n", i + 1, filelist->count, filelist->filenames[i]);
        
        // Construct full file path
        char filepath[MAX_FILENAME_LEN * 2];
        snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, filelist->filenames[i]);

        // Read file content
        char *content = read_file_content(filepath);
        if (!content) continue;

        int word_count = 0;
        Tokenizer* tokenizer = tokenizer_create(content);
        char* token;
        
        // Tokenize and process each token
        while ((token = tokenizer_next_token(tokenizer)) != NULL) {
            word_count++;
            process_token(token, i, index, stopwords);
        }
        
        // Clean up tokenizer
        tokenizer_free(tokenizer);
        
        // Store document length (avoid division by zero)
        filelist->doc_lengths[i] = (word_count > 0) ? word_count : 1;
        free(content);
    }
    return index;
}