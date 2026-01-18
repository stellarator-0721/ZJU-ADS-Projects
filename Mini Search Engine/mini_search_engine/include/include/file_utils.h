#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "common.h"

// File list structure to store information about all files in a directory
typedef struct {
    char **filenames;    // Array of filenames
    int *doc_lengths;    // Length of each document (word count)
    int count;           // Number of files
} FileList;

// Function declarations
FileList* get_files_in_dir(const char *path);        // Get all files in directory
char* read_file_content(const char *filepath);       // Read file content
void free_filelist(FileList *filelist);              // Free file list memory

#endif