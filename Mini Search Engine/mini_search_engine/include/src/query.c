#include "query.h"
#include "../lib/stmr.h"
#include "hash_table.h"
#include <stdbool.h>

// Structure to store query results with scoring information
typedef struct QueryResult {
    int doc_id;        // Document ID
    double score;      // Score for ranking (raw frequency or normalized frequency)
    int raw_freq;      // Raw frequency count
} QueryResult;

/**
 * Compare function for sorting query results by score (descending order)
 * @param a First query result to compare
 * @param b Second query result to compare
 * @return Negative if a should come after b, positive if a should come before b, 0 if equal
 */
int compare_results(const void *a, const void *b) {
    double score_a = ((QueryResult*)a)->score;
    double score_b = ((QueryResult*)b)->score;
    if (score_b > score_a) return 1;    // b has higher score, should come first
    if (score_b < score_a) return -1;   // a has higher score, should come first
    return 0;                           // scores are equal
}

/**
 * Process AND query - find documents containing ALL query terms
 * @param terms Array of query terms
 * @param term_count Number of terms
 * @param index Inverted index to search
 * @param filelist File list for document names
 */
void process_and_query(char **terms, int term_count, InvertedIndex *index, FileList *filelist) {
    if (term_count == 0) return;
    
    // Start with posting list of first term
    DocNode *list1 = ht_search(index, terms[0]);
    if (!list1) {
        printf("No documents contain all query terms.\n");
        return;
    }
    
    // Create hash table to track documents that contain first term
    HashTable *results_ht = ht_create(100);
    DocNode *current = list1;
    while (current) {
        char key[20];
        snprintf(key, sizeof(key), "%d", current->doc_id);
        ht_insert(results_ht, key, (void*)1);
        current = current->next;
    }
    
    // Intersect with posting lists of remaining terms
    for (int i = 1; i < term_count; ++i) {
        DocNode *list_i = ht_search(index, terms[i]);
        HashTable *next_results_ht = ht_create(100);
        current = list_i;
        
        // Keep only documents that are in both current results and this term's list
        while(current) {
            char key[20];
            snprintf(key, sizeof(key), "%d", current->doc_id);
            if (ht_search(results_ht, key)) {
                ht_insert(next_results_ht, key, (void*)1);
            }
            current = current->next;
        }
        
        // Update results and free old hash table
        ht_free(results_ht, NULL);
        results_ht = next_results_ht;
    }
    
    // Display results
    printf("Found documents containing ALL terms:\n");
    int found = 0;
    for (int i = 0; i < results_ht->size; ++i) {
        HashNode *node = results_ht->table[i];
        while (node) {
            int doc_id = atoi(node->key);
            printf("- %s\n", filelist->filenames[doc_id]);
            found = 1;
            node = node->next;
        }
    }
    
    if (!found) {
        printf("No documents contain all query terms.\n");
    }
    
    ht_free(results_ht, NULL);
}

/**
 * Process OR query - find documents containing ANY query term
 * @param terms Array of query terms
 * @param term_count Number of terms
 * @param threshold Frequency threshold
 * @param index Inverted index to search
 * @param filelist File list for document names and lengths
 * @param normalize Whether to use normalized frequency
 */
void process_or_query(char **terms, int term_count, int threshold, InvertedIndex *index, FileList *filelist, bool normalize) {
    HashTable *results_ht = ht_create(100);
    
    // Aggregate frequencies from all query terms
    for (int i = 0; i < term_count; i++) {
        DocNode *list = ht_search(index, terms[i]);
        DocNode *current = list;
        while(current) {
            char key[20];
            snprintf(key, sizeof(key), "%d", current->doc_id);
            int* score = ht_search(results_ht, key);
            if (score) {
                // Document already exists - add frequency
                (*score) += current->frequency;
            } else {
                // New document - initialize frequency
                int *new_score = malloc(sizeof(int));
                *new_score = current->frequency;
                ht_insert(results_ht, key, new_score);
            }
            current = current->next;
        }
    }

    // Count total and filtered results
    int total_found_count = 0;
    int filtered_count = 0;
    for(int i = 0; i < results_ht->size; ++i) {
        HashNode *node = results_ht->table[i];
        while(node) {
            total_found_count++;
            int doc_id = atoi(node->key);
            int total_freq = *(int*)node->value;
            bool meets_threshold;
            
            // Apply threshold check (raw or normalized)
            if (normalize) {
                double norm_freq_per_10000 = (double)total_freq / filelist->doc_lengths[doc_id] * 10000.0;
                meets_threshold = (norm_freq_per_10000 >= threshold);
            } else {
                meets_threshold = (total_freq >= threshold);
            }
            
            if (meets_threshold) {
                filtered_count++;
            }
            node = node->next;
        }
    }

    // Handle no results cases
    if (total_found_count == 0) {
        printf("No documents found for the given query.\n");
        ht_free(results_ht, free);
        return;
    }
    
    if (filtered_count == 0) {
        printf("No documents met the threshold (out of %d total matching documents).\n", total_found_count);
        ht_free(results_ht, free);
        return;
    }
    
    // Create array of filtered results for sorting
    QueryResult *res_array = malloc(filtered_count * sizeof(QueryResult));
    int idx = 0;
    for (int i = 0; i < results_ht->size; ++i) {
        HashNode *node = results_ht->table[i];
        while (node) {
            int doc_id = atoi(node->key);
            int total_freq = *(int*)node->value;
            bool meets_threshold;
            double score = 0.0;
            
            // Calculate score based on normalization setting
            if (normalize) {
                score = (double)total_freq / filelist->doc_lengths[doc_id];
                meets_threshold = (score * 10000.0 >= threshold);
            } else {
                score = (double)total_freq;
                meets_threshold = (total_freq >= threshold);
            }

            if (meets_threshold) {
                res_array[idx].doc_id = doc_id;
                res_array[idx].score = score;
                res_array[idx].raw_freq = total_freq;
                idx++;
            }
            node = node->next;
        }
    }

    // Sort results by score (descending)
    qsort(res_array, filtered_count, sizeof(QueryResult), compare_results);

    // Display results
    if (normalize) {
        printf("Found documents containing ANY term (ranked by normalized frequency, threshold >= %d per 10000 words):\n", threshold);
    } else {
        printf("Found documents containing ANY term (ranked by raw frequency, threshold >= %d):\n", threshold);
    }
    
    for (int i = 0; i < filtered_count; ++i) {
        if (normalize) {
            printf("- %s (Total Freq: %d, Norm Score: %.6f)\n", 
                   filelist->filenames[res_array[i].doc_id], 
                   res_array[i].raw_freq, 
                   res_array[i].score);
        } else {
            printf("- %s (Total Frequency: %d)\n", 
                   filelist->filenames[res_array[i].doc_id], 
                   res_array[i].raw_freq);
        }
    }
    
    // Display filtering statistics
    double filter_rate = (double)filtered_count / total_found_count * 100.0;
    printf("\nFiltering Rate: %d / %d = %.2f%%\n", filtered_count, total_found_count, filter_rate);

    free(res_array);
    ht_free(results_ht, free);
}

/**
 * Main query processing function
 * @param query_str The query string from user
 * @param threshold Frequency threshold
 * @param index Inverted index to search
 * @param filelist File list for document information
 * @param stopwords Stop words hash table
 * @param normalize Whether to use normalized frequency
 */
void process_query(const char *query_str, int threshold, InvertedIndex *index, FileList *filelist, HashTable *stopwords, int normalize) {
    char *query_copy = strdup(query_str);
    char *to_free = query_copy;

    char *initial_terms[MAX_WORD_LEN];
    int initial_term_count = 0;

    // Tokenize query
    const char *delimiters = " \t\n\r";
    char *token = strtok(query_copy, delimiters);

    // Process each token: lowercase and filter stop words
    while (token != NULL && initial_term_count < MAX_WORD_LEN) {
        for (int i = 0; token[i]; i++) token[i] = tolower(token[i]);
        if (ht_search(stopwords, token)) {
            printf("Info: Query term '%s' is a stopword and has been ignored.\n", token);
        } else {
            initial_terms[initial_term_count++] = token;
        }
        token = strtok(NULL, delimiters);
    }
    
    // Check if all terms were stop words
    if (initial_term_count == 0) {
        printf("All query terms were stopwords. Please enter a different query.\n");
        free(to_free);
        return;
    }

    // Stem remaining terms
    char *terms[MAX_WORD_LEN];
    int term_count = 0;
    for(int i = 0; i < initial_term_count; i++) {
        int end = stem(initial_terms[i], 0, strlen(initial_terms[i]) - 1);
        initial_terms[i][end + 1] = '\0';
        if (strlen(initial_terms[i]) > 0) {
            terms[term_count++] = initial_terms[i];
        }
    }

    // Handle single term query
    if (term_count == 1) {
        if (normalize) {
            printf("\n--- Query Results for single term: \"%s\" (threshold >= %d per 10000 words) ---\n", terms[0], threshold);
        } else {
            printf("\n--- Query Results for single term: \"%s\" (threshold >= %d) ---\n", terms[0], threshold);
        }
        
        DocNode *list = ht_search(index, terms[0]);

        if (!list) {
            printf("Term not found in any document.\n");
        } else {
            int total_found_count = 0;
            int filtered_count = 0;
            DocNode *current = list;
            
            // Count total and filtered documents
            while (current) {
                total_found_count++;
                bool meets_threshold;
                if (normalize) {
                    double norm_freq_per_10000 = (double)current->frequency / filelist->doc_lengths[current->doc_id] * 10000.0;
                    meets_threshold = (norm_freq_per_10000 >= threshold);
                } else {
                    meets_threshold = (current->frequency >= threshold);
                }
                if (meets_threshold) {
                    filtered_count++;
                }
                current = current->next;
            }

            if (filtered_count == 0) {
                printf("Term found in %d document(s), but none meet the frequency threshold.\n", total_found_count);
            } else {
                printf("Found in the following documents (ranked by %s frequency):\n", normalize ? "normalized" : "raw");
                QueryResult *sorted_results = malloc(filtered_count * sizeof(QueryResult));
                current = list;
                int i = 0;
                
                // Collect and sort results
                while (current) {
                    bool meets_threshold;
                    double score = 0.0;
                    if (normalize) {
                        score = (double)current->frequency / filelist->doc_lengths[current->doc_id];
                        meets_threshold = (score * 10000.0 >= threshold);
                    } else {
                        score = (double)current->frequency;
                        meets_threshold = (current->frequency >= threshold);
                    }

                    if (meets_threshold) {
                        sorted_results[i].doc_id = current->doc_id;
                        sorted_results[i].score = score;
                        sorted_results[i].raw_freq = current->frequency;
                        i++;
                    }
                    current = current->next;
                }
                
                qsort(sorted_results, filtered_count, sizeof(QueryResult), compare_results);
                
                // Display sorted results
                for (i = 0; i < filtered_count; ++i) {
                     if (normalize) {
                        printf("- %s (Freq: %d, Norm Score: %.6f)\n", 
                               filelist->filenames[sorted_results[i].doc_id], 
                               sorted_results[i].raw_freq, 
                               sorted_results[i].score);
                    } else {
                        printf("- %s (Frequency: %d)\n", 
                               filelist->filenames[sorted_results[i].doc_id], 
                               sorted_results[i].raw_freq);
                    }
                }
                free(sorted_results);
                
                // Display filtering statistics
                double filter_rate = (double)filtered_count / total_found_count * 100.0;
                printf("\nFiltering Rate: %d / %d = %.2f%%\n", filtered_count, total_found_count, filter_rate);
            }
        }
    } 
    // Handle multi-term query
    else if (term_count > 1) {
        // Reconstruct original query without stop words for display
        char original_query_no_stopwords[1024] = {0};
        for(int i=0; i < initial_term_count; ++i) {
            strcat(original_query_no_stopwords, initial_terms[i]);
            if (i < initial_term_count - 1) strcat(original_query_no_stopwords, " ");
        }
        
        printf("\n--- Query Results for phrase: \"%s\" ---\n", original_query_no_stopwords);
        printf("Stemmed terms: ");
        for(int i=0; i<term_count; ++i) printf("%s ", terms[i]);
        printf("\n\n");
        
        // Run both AND and OR queries for comparison
        printf("--- High Threshold (AND Query) ---\n");
        process_and_query(terms, term_count, index, filelist);
        printf("\n");
        printf("--- Low Threshold (OR Query) ---\n");
        process_or_query(terms, term_count, threshold, index, filelist, normalize);
    }
    
    printf("--- End of Query ---\n");
    free(to_free);
}