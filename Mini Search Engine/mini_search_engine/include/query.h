#ifndef QUERY_H
#define QUERY_H

#include "inverted_index.h"
#include "file_utils.h"
#include "hash_table.h"

// Query processing function
void process_query(const char *query_str, int threshold, InvertedIndex *index, 
                  FileList *filelist, HashTable *stopwords, int normalize);

#endif