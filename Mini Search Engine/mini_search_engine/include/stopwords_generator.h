#ifndef STOPWORDS_GENERATOR_H
#define STOPWORDS_GENERATOR_H

// Stop words generator functions
void run_raw_word_count(const char* data_dir, const char* freq_file_out);  // Count raw word frequencies

int run_stopword_analysis(const char* freq_file_in, const char* standard_stopwords_in, 
                         const char* final_stopwords_out);  // Analyze and generate final stop words

#endif