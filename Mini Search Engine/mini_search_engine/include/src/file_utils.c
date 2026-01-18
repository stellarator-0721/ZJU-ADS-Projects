#include "file_utils.h"
#include <sys/stat.h>

/**
 * Get all regular files in the specified directory
 * @param path Directory path to scan
 * @return FileList structure containing filenames, or NULL on error
 */
FileList* get_files_in_dir(const char *path) {
    DIR *d;
    struct dirent *dir;
    
    // Allocate memory for file list structure
    FileList *filelist = malloc(sizeof(FileList));
    if (!filelist) {
        perror("Failed to allocate memory for filelist");
        return NULL;
    }
    
    // Initialize file list
    filelist->count = 0;
    filelist->filenames = NULL;
    filelist->doc_lengths = NULL;

    // Open directory
    d = opendir(path);
    if (d) {
        // Iterate through directory entries
        while ((dir = readdir(d)) != NULL) {
            // Skip "." and ".." entries
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                continue;
            }
            
            // Construct full file path
            char full_path[MAX_FILENAME_LEN * 2];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->d_name);

            // Check if it's a regular file
            struct stat s;
            if (stat(full_path, &s) == 0) {
                if (S_ISREG(s.st_mode)) {
                    // Increase file count and reallocate filename array
                    filelist->count++;
                    filelist->filenames = realloc(filelist->filenames, filelist->count * sizeof(char*));
                    if (!filelist->filenames) {
                        perror("Failed to reallocate memory for filenames");
                        closedir(d);
                        free_filelist(filelist);
                        return NULL;
                    }
                    // Store filename
                    filelist->filenames[filelist->count - 1] = strdup(dir->d_name);
                }
            }
        }
        closedir(d);
    } else {
        perror("Could not open directory");
        free(filelist);
        return NULL;
    }
    return filelist;
}

/**
 * Free memory allocated for a FileList structure
 * @param filelist Pointer to FileList structure to free
 */
void free_filelist(FileList *filelist) {
    if (!filelist) return;
    
    // Free all filename strings
    for (int i = 0; i < filelist->count; ++i) {
        free(filelist->filenames[i]);
    }
    
    // Free arrays and structure itself
    free(filelist->filenames);
    free(filelist->doc_lengths);
    free(filelist);
}

/**
 * Read entire file content into a string
 * @param filepath Path to the file to read
 * @return Dynamically allocated string containing file content, or NULL on error
 */
char* read_file_content(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer for file content
    char *buffer = malloc(length + 1);
    if (buffer) {
        // Read file content
        if (fread(buffer, 1, length, file) != (size_t)length) {
            fprintf(stderr, "Error reading file: %s\n", filepath);
            free(buffer);
            fclose(file);
            return NULL;
        }
        // Null-terminate the string
        buffer[length] = '\0';
    }
    fclose(file);
    return buffer;
}