#include "common.h"
#include "file_utils.h"
#include "inverted_index.h"
#include "query.h"
#include <stdbool.h>

/**
 * Main function for the Mini Search Engine
 * This program:
 * 1. Loads documents from a directory
 * 2. Builds an inverted index
 * 3. Provides interactive query interface
 * 4. Cleans up resources on exit
 */
int main(int argc, char *argv[]) {
    // Define file paths
    const char *docs_path = "data/shakespeare_texts";      // Directory containing documents
    const char *stopwords_path = "data/stopwords.txt";     // Stop words file

    InvertedIndex *index = NULL;    // Inverted index structure
    FileList *filelist = NULL;      // List of documents

    printf("Mini Search Engine\n");
    printf("==================\n");

    // Step 1: Get list of documents in directory
    filelist = get_files_in_dir(docs_path);
    if (!filelist || filelist->count == 0) {
        fprintf(stderr, "Error: No text files found in '%s'\n", docs_path);
        return 1;
    }
    printf("Found %d documents to process.\n\n", filelist->count);

    // Step 2: Build inverted index
    printf("Building inverted index... (This may take a moment)\n");
    HashTable *stopwords = load_stopwords(stopwords_path);
    if (!stopwords) {
        free_filelist(filelist);
        return 1;
    }
    index = build_inverted_index(docs_path, stopwords, filelist);
    printf("Inverted index built successfully.\n\n");

    // Step 3: Interactive query interface
    printf("Entering interactive query mode. Type 'exit' to quit.\n");
    printf("Usage examples:\n");
    printf("  noble                    # Search for 'noble' with default threshold\n");
    printf("  noble : 5                # Search for 'noble' with threshold 5\n");
    printf("  noble : 5 norm           # Sort by normalized frequency (per 10000 words)\n");
    
    char query_buffer[1024];  // Buffer for user input
    while (1) {
        printf("> ");
        // Read user input
        if (fgets(query_buffer, sizeof(query_buffer), stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove newline character
        query_buffer[strcspn(query_buffer, "\n")] = 0;

        // Check for exit command
        if (strcmp(query_buffer, "exit") == 0) {
            break;
        }

        // Process non-empty queries
        if (strlen(query_buffer) > 0) {
            char *query_part = query_buffer;
            int threshold = 1;      // Default threshold
            bool normalize = false; // Default sorting method

            // Check for normalization flag
            char *norm_flag = strstr(query_buffer, "norm");
            if (norm_flag) {
                int len_before = norm_flag - query_buffer;
                // Ensure "norm" is a standalone word
                if ((len_before == 0 || isspace(*(norm_flag - 1))) && 
                    (*(norm_flag + 4) == '\0' || isspace(*(norm_flag + 4)))) {
                    normalize = true;
                    *norm_flag = '\0';  // Remove "norm" from query string
                }
            }

            // Parse threshold (if specified)
            char *colon = strrchr(query_part, ':');
            if (colon) {
                *colon = '\0';  // Split query and threshold
                char *threshold_part = colon + 1;
                threshold = atoi(threshold_part);
                if (threshold < 1) {
                    threshold = 1;  // Ensure minimum threshold
                }
            }
            
            // Trim trailing whitespace from query
            int len = strlen(query_part);
            while(len > 0 && isspace(query_part[len-1])) {
                query_part[--len] = '\0';
            }

            // Process query if not empty after parsing
            if(strlen(query_part) > 0) {
                 process_query(query_part, threshold, index, filelist, stopwords, normalize);
            }
        }
    }
 
    // Step 4: Clean up resources
    printf("Cleaning up resources...\n");
    free_inverted_index(index);     // Free inverted index and posting lists
    ht_free(stopwords, NULL);       // Free stop words hash table
    free_filelist(filelist);        // Free file list
    printf("Done.\n");

    return 0;
}