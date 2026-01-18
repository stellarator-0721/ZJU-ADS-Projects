#include "stopwords_generator.h"
#include <stdio.h>
#include <unistd.h>

/**
 * Main function for generating stop words
 * This program:
 * 1. Counts word frequencies from Shakespeare texts
 * 2. Combines with standard stop words
 * 3. Generates a final stop words list
 * 4. Cleans up temporary files
 */
int main() {
    // Define file paths
    const char* data_dir = "data/shakespeare_texts";          // Directory containing Shakespeare texts
    const char* standard_stopwords_in = "data/stopwords_standard.txt"; // Standard stop words file
    const char* raw_freq_out = "raw_word_frequency.txt";      // Temporary file for raw frequency data
    const char* final_stopwords_out = "data/stopwords.txt";   // Final output stop words file

    // Step 1: Count word frequencies from Shakespeare texts
    printf("Counting word frequencies from: %s\n", data_dir);
    run_raw_word_count(data_dir, raw_freq_out);

    // Step 2: Check if final stop words file already exists
    if (access(final_stopwords_out, F_OK) == 0) {
        printf("Target file '%s' already exists.\n", final_stopwords_out);
        printf("Skipping generation.\n");
    } else {
        // Step 3: Generate final stop words by analyzing frequencies
        printf("Generating stop words...\n");
        run_stopword_analysis(raw_freq_out, standard_stopwords_in, final_stopwords_out);
        printf("=================================\n");
        printf("Successfully generated stopwords file: %s\n", final_stopwords_out);
    }
    
    // Step 4: Clean up temporary raw frequency file
    printf("Cleaning up temporary file: %s\n", raw_freq_out);
    remove(raw_freq_out);

    return 0;
}