#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "stopwords_generator.h"

#define MAX_WORD_LENGTH 100
#define MAX_WORDS 1000

// Structure to store word frequency information
typedef struct {
    char word[MAX_WORD_LENGTH];  // The word itself
    int frequency;               // Frequency count in the corpus
    double percentage;           // Percentage of total words
    int rank;                    // Rank by frequency (1 = most frequent)
} WordInfo;

// Function declarations
static int read_enhanced_frequency_file(const char* filename, WordInfo words[], int max_words);
static int is_likely_stopword(const char* word, int frequency, double percentage, int rank);

/**
 * Main function for stopword analysis and generation
 * Combines frequency-based analysis with standard stopwords list
 * @param freq_file_in Input file with word frequency data
 * @param standard_stopwords_in Standard stopwords list file
 * @param final_stopwords_out Output file for final stopwords list
 * @return 0 on success, 1 on error
 */
int run_stopword_analysis(const char* freq_file_in, const char* standard_stopwords_in, const char* final_stopwords_out) {
    WordInfo words[MAX_WORDS];
    int word_count;
    
    printf("Initializing stopword analysis...\n");
    printf("Reading from intermediate file: %s\n", freq_file_in);
    
    // Read word frequency data from file
    word_count = read_enhanced_frequency_file(freq_file_in, words, MAX_WORDS);
    if (word_count == 0) {
        fprintf(stderr, "Error: Could not read frequency file '%s' or file is empty.\n", freq_file_in);
        return 1;
    }
    
    // Open output file for writing
    FILE* final_file = fopen(final_stopwords_out, "w");
    if (!final_file) {
        fprintf(stderr, "Error: Could not create final stopwords file '%s'.\n", final_stopwords_out);
        return 1;
    }

    // Step 1: Write stopwords identified through frequency analysis
    int analyzed_count = 0;
    for (int i = 0; i < word_count; i++) {
        if (is_likely_stopword(words[i].word, words[i].frequency, words[i].percentage, words[i].rank)) {
            fprintf(final_file, "%s\n", words[i].word);
            analyzed_count++;
        }
    }
    printf("Wrote %d stopwords based on frequency analysis.\n", analyzed_count);

    // Step 2: Append standard stopwords from predefined list
    FILE* standard_file = fopen(standard_stopwords_in, "r");
    char buffer[MAX_WORD_LENGTH];
    int standard_count = 0;
    if (standard_file) {
        while (fgets(buffer, sizeof(buffer), standard_file)) {
            buffer[strcspn(buffer, "\n")] = 0;  // Remove newline
            if (strlen(buffer) > 0) {
                fprintf(final_file, "%s\n", buffer);
                standard_count++;
            }
        }
        fclose(standard_file);
    }
    printf("Appended %d words from the standard stopword list.\n", standard_count);
    
    fclose(final_file);
    
    printf("Total stopwords in final list: %d\n", analyzed_count + standard_count);
    printf("Stopword analysis and merge module finished.\n");
    return 0;
}

/**
 * Read enhanced frequency data from file
 * Parses formatted frequency data with rank, word, frequency, and percentage
 * @param filename Input file name
 * @param words Array to store WordInfo structures
 * @param max_words Maximum number of words to read
 * @return Number of words successfully read
 */
static int read_enhanced_frequency_file(const char* filename, WordInfo words[], int max_words) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    
    char line[512];
    int count = 0;
    bool in_data_section = false;
    
    // Read file line by line
    while (fgets(line, sizeof(line), file) && count < max_words) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        
        // Look for start of data section
        if (strstr(line, "------------------------------------------------") != NULL) {
            in_data_section = true;
            continue;
        }
        
        // Parse data lines in the data section
        if (in_data_section && strlen(line) > 10) {
            int rank;
            char word[MAX_WORD_LENGTH];
            int frequency;
            double percentage;
            
            // Parse formatted line: "Rank Word Frequency Percentage%"
            if (sscanf(line, "%d %s %d %lf%%", &rank, word, &frequency, &percentage) == 4) {
                 if (strlen(word) > 0 && rank > 0) {
                    strcpy(words[count].word, word);
                    words[count].frequency = frequency;
                    words[count].percentage = percentage;
                    words[count].rank = rank;
                    count++;
                }
            }
        }
        
        // Stop reading at end of data section
        if (in_data_section && count > 0 && (strlen(line) == 0 || strstr(line, "===") != NULL)) {
            break;
        }
    }
    
    fclose(file);
    return count;
}

/**
 * Determine if a word is likely to be a stopword based on various criteria
 * @param word The word to check
 * @param frequency Frequency of the word in corpus
 * @param percentage Percentage of total words
 * @param rank Rank by frequency (1 = most frequent)
 * @return 1 if likely stopword, 0 otherwise
 */
static int is_likely_stopword(const char* word, int frequency, double percentage, int rank) {
    // High rank and high percentage indicates very common word
    if (rank <= 30 && percentage > 0.08) {
        return 1;
    }
    
    // Short words with high frequency
    if (strlen(word) <= 4 && percentage > 0.05) {
        return 1;
    }
    
    // Very short words (1-2 characters) with moderate frequency
    if (strlen(word) <= 2 && percentage > 0.01) {
        return 1;
    }
    
    // Common functional words (pronouns, prepositions, conjunctions, etc.)
    const char* common_functional_words[] = {
        "i", "you", "he", "she", "it", "we", "they", "me", "him", "her", 
        "us", "them", "my", "your", "his", "its", "our", "their", "mine", "yours",
        "this", "that", "these", "those", "to", "with", "upon", "within", "without", 
        "under", "over", "between", "among", "through", "during", "before", "after",
        "and", "but", "or", "nor", "for", "yet", "so", "though", "although", "will", 
        "would", "shall", "should", "can", "could", "may", "might", "must", "have", 
        "has", "had", "do", "does", "did", "say", "said", "see", "saw", "come", 
        "came", "go", "went", "know", "knew", "think", "thought", "very", "more", 
        "most", "some", "any", "all", "such", "only", "just", NULL
    };
    for (int i = 0; common_functional_words[i] != NULL; i++) {
        if (strcmp(word, common_functional_words[i]) == 0) {
            return 1;
        }
    }
    
    // Shakespeare-specific common words
    const char* shakespeare_specific[] = {
        "thou", "thy", "thee", "thine", "hath", "doth", "hast", "art", 
        "'tis", "exeunt", "enter", "pray", "nay", "aye", "forsooth",
        "wherefore", "hark", "behold", "alas", "adieu", "oft", "ere", NULL
    };
    for (int i = 0; shakespeare_specific[i] != NULL; i++) {
        if (strcmp(word, shakespeare_specific[i]) == 0) {
            return 1;
        }
    }
    
    // Words with apostrophes (contractions) that are common
    if (strchr(word, '\'') != NULL && percentage > 0.03) {
        return 1;
    }
    
    // High-frequency abstract concepts that might not carry specific meaning
    if (percentage > 0.1 && rank <= 50) {
        const char* abstract_concepts[] = {
            "time", "love", "death", "life", "heart", "world", "hand", "eye",
            "day", "night", "man", "men", "way", "thing", "word", "peace",
            "honour", "grace", "fear", "hope", "truth", "beauty", NULL
        };
        for (int i = 0; abstract_concepts[i] != NULL; i++) {
            if (strcmp(word, abstract_concepts[i]) == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "stopwords_generator.h"

#define MAX_WORD_LENGTH 100
#define MAX_WORDS 1000

// Structure to store word frequency information
typedef struct {
    char word[MAX_WORD_LENGTH];  // The word itself
    int frequency;               // Frequency count in the corpus
    double percentage;           // Percentage of total words
    int rank;                    // Rank by frequency (1 = most frequent)
} WordInfo;

// Function declarations
static int read_enhanced_frequency_file(const char* filename, WordInfo words[], int max_words);
static int is_likely_stopword(const char* word, int frequency, double percentage, int rank);

/**
 * Main function for stopword analysis and generation
 * Combines frequency-based analysis with standard stopwords list
 * @param freq_file_in Input file with word frequency data
 * @param standard_stopwords_in Standard stopwords list file
 * @param final_stopwords_out Output file for final stopwords list
 * @return 0 on success, 1 on error
 */
int run_stopword_analysis(const char* freq_file_in, const char* standard_stopwords_in, const char* final_stopwords_out) {
    WordInfo words[MAX_WORDS];
    int word_count;
    
    printf("Initializing stopword analysis...\n");
    printf("Reading from intermediate file: %s\n", freq_file_in);
    
    // Read word frequency data from file
    word_count = read_enhanced_frequency_file(freq_file_in, words, MAX_WORDS);
    if (word_count == 0) {
        fprintf(stderr, "Error: Could not read frequency file '%s' or file is empty.\n", freq_file_in);
        return 1;
    }
    
    // Open output file for writing
    FILE* final_file = fopen(final_stopwords_out, "w");
    if (!final_file) {
        fprintf(stderr, "Error: Could not create final stopwords file '%s'.\n", final_stopwords_out);
        return 1;
    }

    // Step 1: Write stopwords identified through frequency analysis
    int analyzed_count = 0;
    for (int i = 0; i < word_count; i++) {
        if (is_likely_stopword(words[i].word, words[i].frequency, words[i].percentage, words[i].rank)) {
            fprintf(final_file, "%s\n", words[i].word);
            analyzed_count++;
        }
    }
    printf("Wrote %d stopwords based on frequency analysis.\n", analyzed_count);

    // Step 2: Append standard stopwords from predefined list
    FILE* standard_file = fopen(standard_stopwords_in, "r");
    char buffer[MAX_WORD_LENGTH];
    int standard_count = 0;
    if (standard_file) {
        while (fgets(buffer, sizeof(buffer), standard_file)) {
            buffer[strcspn(buffer, "\n")] = 0;  // Remove newline
            if (strlen(buffer) > 0) {
                fprintf(final_file, "%s\n", buffer);
                standard_count++;
            }
        }
        fclose(standard_file);
    }
    printf("Appended %d words from the standard stopword list.\n", standard_count);
    
    fclose(final_file);
    
    printf("Total stopwords in final list: %d\n", analyzed_count + standard_count);
    printf("Stopword analysis and merge module finished.\n");
    return 0;
}

/**
 * Read enhanced frequency data from file
 * Parses formatted frequency data with rank, word, frequency, and percentage
 * @param filename Input file name
 * @param words Array to store WordInfo structures
 * @param max_words Maximum number of words to read
 * @return Number of words successfully read
 */
static int read_enhanced_frequency_file(const char* filename, WordInfo words[], int max_words) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    
    char line[512];
    int count = 0;
    bool in_data_section = false;
    
    // Read file line by line
    while (fgets(line, sizeof(line), file) && count < max_words) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        
        // Look for start of data section
        if (strstr(line, "------------------------------------------------") != NULL) {
            in_data_section = true;
            continue;
        }
        
        // Parse data lines in the data section
        if (in_data_section && strlen(line) > 10) {
            int rank;
            char word[MAX_WORD_LENGTH];
            int frequency;
            double percentage;
            
            // Parse formatted line: "Rank Word Frequency Percentage%"
            if (sscanf(line, "%d %s %d %lf%%", &rank, word, &frequency, &percentage) == 4) {
                 if (strlen(word) > 0 && rank > 0) {
                    strcpy(words[count].word, word);
                    words[count].frequency = frequency;
                    words[count].percentage = percentage;
                    words[count].rank = rank;
                    count++;
                }
            }
        }
        
        // Stop reading at end of data section
        if (in_data_section && count > 0 && (strlen(line) == 0 || strstr(line, "===") != NULL)) {
            break;
        }
    }
    
    fclose(file);
    return count;
}

/**
 * Determine if a word is likely to be a stopword based on various criteria
 * @param word The word to check
 * @param frequency Frequency of the word in corpus
 * @param percentage Percentage of total words
 * @param rank Rank by frequency (1 = most frequent)
 * @return 1 if likely stopword, 0 otherwise
 */
static int is_likely_stopword(const char* word, int frequency, double percentage, int rank) {
    // High rank and high percentage indicates very common word
    if (rank <= 30 && percentage > 0.08) {
        return 1;
    }
    
    // Short words with high frequency
    if (strlen(word) <= 4 && percentage > 0.05) {
        return 1;
    }
    
    // Very short words (1-2 characters) with moderate frequency
    if (strlen(word) <= 2 && percentage > 0.01) {
        return 1;
    }
    
    // Common functional words (pronouns, prepositions, conjunctions, etc.)
    const char* common_functional_words[] = {
        "i", "you", "he", "she", "it", "we", "they", "me", "him", "her", 
        "us", "them", "my", "your", "his", "its", "our", "their", "mine", "yours",
        "this", "that", "these", "those", "to", "with", "upon", "within", "without", 
        "under", "over", "between", "among", "through", "during", "before", "after",
        "and", "but", "or", "nor", "for", "yet", "so", "though", "although", "will", 
        "would", "shall", "should", "can", "could", "may", "might", "must", "have", 
        "has", "had", "do", "does", "did", "say", "said", "see", "saw", "come", 
        "came", "go", "went", "know", "knew", "think", "thought", "very", "more", 
        "most", "some", "any", "all", "such", "only", "just", NULL
    };
    for (int i = 0; common_functional_words[i] != NULL; i++) {
        if (strcmp(word, common_functional_words[i]) == 0) {
            return 1;
        }
    }
    
    // Shakespeare-specific common words
    const char* shakespeare_specific[] = {
        "thou", "thy", "thee", "thine", "hath", "doth", "hast", "art", 
        "'tis", "exeunt", "enter", "pray", "nay", "aye", "forsooth",
        "wherefore", "hark", "behold", "alas", "adieu", "oft", "ere", NULL
    };
    for (int i = 0; shakespeare_specific[i] != NULL; i++) {
        if (strcmp(word, shakespeare_specific[i]) == 0) {
            return 1;
        }
    }
    
    // Words with apostrophes (contractions) that are common
    if (strchr(word, '\'') != NULL && percentage > 0.03) {
        return 1;
    }
    
    // High-frequency abstract concepts that might not carry specific meaning
    if (percentage > 0.1 && rank <= 50) {
        const char* abstract_concepts[] = {
            "time", "love", "death", "life", "heart", "world", "hand", "eye",
            "day", "night", "man", "men", "way", "thing", "word", "peace",
            "honour", "grace", "fear", "hope", "truth", "beauty", NULL
        };
        for (int i = 0; abstract_concepts[i] != NULL; i++) {
            if (strcmp(word, abstract_concepts[i]) == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}