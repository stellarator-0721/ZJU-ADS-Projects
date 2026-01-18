#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_WORD_LENGTH 100
#define MAX_WORDS 100000
#define HASH_SIZE 100000

// 单词频率结构
typedef struct WordNode {
    char word[MAX_WORD_LENGTH];
    int frequency;
    struct WordNode* next;
} WordNode;

// 哈希表
WordNode* word_table[HASH_SIZE];
int total_words = 0;
int unique_words = 0;

// 字符串转换为小写
void to_lower_extended(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
    }
}

// 哈希函数
unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

// 检查字符是否为字母
int is_alpha_extended(char c) {
    // 基本英文字母
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        return 1;
    }
    
    // 法语重音字符的基本范围
    if ((unsigned char)c >= 0xC0 && (unsigned char)c <= 0xFF) {
        return 1;
    }
    
    // 连字符和撇号
    if (c == '-' || c == '\'' || c == 0xE9) {
        return 1;
    }
    
    return 0;
}

// 读取文件内容
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Cannot open file: %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (length <= 0) {
        fclose(file);
        return NULL;
    }
    
    char* buffer = malloc(length + 1);
    if (buffer) {
        size_t read_bytes = fread(buffer, 1, length, file);
        buffer[read_bytes] = '\0';
    }
    
    fclose(file);
    return buffer;
}

// 添加单词到哈希表
void add_word(const char* word) {
    if (strlen(word) == 0 || strlen(word) == 1) return;
    
    char processed_word[MAX_WORD_LENGTH];
    strcpy(processed_word, word);
    to_lower_extended(processed_word);
    
    unsigned int index = hash(processed_word);
    WordNode* current = word_table[index];
    
    // 查找是否已存在
    while (current != NULL) {
        if (strcmp(current->word, processed_word) == 0) {
            current->frequency++;
            return;
        }
        current = current->next;
    }
    
    // 创建新节点
    WordNode* new_node = malloc(sizeof(WordNode));
    if (!new_node) {
        printf("Memory allocation failed!\n");
        return;
    }
    
    strcpy(new_node->word, processed_word);
    new_node->frequency = 1;
    new_node->next = word_table[index];
    word_table[index] = new_node;
    
    unique_words++;
}

// 分词函数
void tokenize_extended(const char* text) {
    int len = strlen(text);
    char current_word[MAX_WORD_LENGTH];
    int word_len = 0;
    int in_word = 0;
    
    for (int i = 0; i <= len; i++) {
        char c = text[i];
        
        if (i < len && is_alpha_extended(c)) {
            if (word_len < MAX_WORD_LENGTH - 1) {
                current_word[word_len++] = c;
            }
            in_word = 1;
        } else {
            if (in_word && word_len > 1) {
                current_word[word_len] = '\0';
                add_word(current_word);
                total_words++;
                
                word_len = 0;
                in_word = 0;
            } else if (in_word) {
                word_len = 0;
                in_word = 0;
            }
        }
    }
}

// 处理单个文件
void process_file(const char* filename) {
    printf("Processing: %s\n", filename);
    
    char* content = read_file(filename);
    if (content) {
        tokenize_extended(content);
        free(content);
    } else {
        printf("Failed to read: %s\n", filename);
    }
}

// 检查是否是文本文件
int is_text_file(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (ext) {
        if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".TXT") == 0) {
            return 1;
        }
    }
    return 1; // 如果没有扩展名，也认为是文本文件
}

// 处理目录中的所有文件
void process_directory(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Error: Cannot open directory: %s\n", dir_path);
        printf("Please check if the directory exists and try again.\n");
        return;
    }
    
    struct dirent* entry;
    int file_count = 0;
    
    printf("Scanning directory: %s\n", dir_path);
    printf("----------------------------------------\n");
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过隐藏文件和当前目录/上级目录
        if (entry->d_name[0] == '.') continue;
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
        
        struct stat path_stat;
        if (stat(filepath, &path_stat) != 0) {
            continue;
        }
        
        if (S_ISREG(path_stat.st_mode)) {
            if (is_text_file(entry->d_name)) {
                process_file(filepath);
                file_count++;
            }
        }
    }
    
    closedir(dir);
    printf("----------------------------------------\n");
    printf("Total files processed: %d\n", file_count);
}

// 比较函数用于排序（按频率降序）
int compare_words(const void* a, const void* b) {
    const WordNode* node_a = *(const WordNode**)a;
    const WordNode* node_b = *(const WordNode**)b;
    return node_b->frequency - node_a->frequency;
}

// 保存结果到文件
void save_results_to_file(const char* filename, WordNode** all_words, int top_n) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Cannot create output file: %s\n", filename);
        return;
    }
    
    fprintf(file, "Shakespeare Word Frequency Analysis\n");
    fprintf(file, "====================================\n");
    fprintf(file, "Total words: %d\n", total_words);
    fprintf(file, "Unique words: %d\n", unique_words);
    fprintf(file, "\nTop %d Most Frequent Words:\n", top_n);
    fprintf(file, "%-6s %-20s %-10s %s\n", "Rank", "Word", "Frequency", "Percentage");
    fprintf(file, "------------------------------------------------\n");
    
    for (int i = 0; i < top_n && i < unique_words; i++) {
        double percentage = (double)all_words[i]->frequency / total_words * 100;
        fprintf(file, "%-6d %-20s %-10d %.4f%%\n", 
                i + 1, all_words[i]->word, all_words[i]->frequency, percentage);
    }
    
    // 添加可能的停用词识别
    fprintf(file, "\nPotential Stop Words (High Frequency):\n");
    fprintf(file, "----------------------------------------\n");
    int stop_word_count = 0;
    for (int i = 0; i < top_n && i < unique_words; i++) {
        double percentage = (double)all_words[i]->frequency / total_words * 100;
        if (percentage > 0.1) { // 频率超过0.1%的可能是停用词
            fprintf(file, "%-20s %-10d %.4f%%\n", 
                    all_words[i]->word, all_words[i]->frequency, percentage);
            stop_word_count++;
        }
    }
    
    fclose(file);
    printf("Results saved to: %s\n", filename);
}

// 显示词频统计结果
void display_word_freq(int top_n) {
    printf("\n=== Word Frequency Statistics ===\n");
    printf("Total words: %d\n", total_words);
    printf("Unique words: %d\n", unique_words);
    
    if (unique_words == 0) {
        printf("No words found in the specified directory!\n");
        return;
    }
    
    // 收集所有单词节点
    WordNode** all_words = malloc(unique_words * sizeof(WordNode*));
    if (!all_words) {
        printf("Memory allocation failed for sorting!\n");
        return;
    }
    
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        WordNode* current = word_table[i];
        while (current != NULL) {
            all_words[count++] = current;
            current = current->next;
        }
    }
    
    // 按频率排序
    qsort(all_words, unique_words, sizeof(WordNode*), compare_words);
    
    // 显示前top_n个单词
    printf("\nTop %d most frequent words:\n", top_n);
    printf("%-6s %-20s %-10s %s\n", "Rank", "Word", "Frequency", "Percentage");
    printf("------------------------------------------------\n");
    
    for (int i = 0; i < top_n && i < unique_words; i++) {
        double percentage = (double)all_words[i]->frequency / total_words * 100;
        printf("%-6d %-20s %-10d %.4f%%\n", 
               i + 1, all_words[i]->word, all_words[i]->frequency, percentage);
    }
    
    // 保存结果到文件
    save_results_to_file("word_frequency_results.txt", all_words, top_n);
    
    // 显示一些统计信息
    printf("\n=== Additional Statistics ===\n");
    printf("Average word frequency: %.2f\n", (double)total_words / unique_words);
    
    // 显示可能的法语单词
    printf("\nPossible French words (with accents):\n");
    int french_examples = 0;
    for (int i = 0; i < unique_words && french_examples < 5; i++) {
        const char* word = all_words[i]->word;
        int might_be_french = 0;
        
        // 检查法语特征
        for (int j = 0; word[j]; j++) {
            unsigned char c = word[j];
            // 检查法语重音字符
            if ((c >= 0xC0 && c <= 0xFF) && 
                c != 0xD7 && c != 0xF7) { // 排除乘除符号
                might_be_french = 1;
                break;
            }
        }
        
        if (might_be_french && all_words[i]->frequency > 1) {
            printf("%-20s %-10d\n", word, all_words[i]->frequency);
            french_examples++;
        }
    }
    
    if (french_examples == 0) {
        printf("No obvious French words detected.\n");
    }
    
    free(all_words);
}

// 释放内存
void cleanup() {
    for (int i = 0; i < HASH_SIZE; i++) {
        WordNode* current = word_table[i];
        while (current != NULL) {
            WordNode* temp = current;
            current = current->next;
            free(temp);
        }
        word_table[i] = NULL;
    }
}

int main() {
    const char* data_dir = "C:/Users/ccy/Desktop/code/project1/shakespeare_texts";
    
    printf("Starting Shakespeare Word Frequency Analysis\n");
    printf("============================================\n");
    printf("Directory: %s\n", data_dir);
    printf("\n");
    
    // 初始化哈希表
    memset(word_table, 0, sizeof(word_table));
    
    // 处理目录
    process_directory(data_dir);
    
    if (total_words > 0) {
        // 显示前100个最常见单词
        display_word_freq(100);
    } else {
        printf("\nNo text files found or no words processed.\n");
        printf("Please check:\n");
        printf("1. The directory exists: %s\n", data_dir);
        printf("2. The directory contains .txt files\n");
        printf("3. You have read permissions for the files\n");
    }
    
    // 清理内存
    cleanup();
    
    printf("\nAnalysis completed.\n");
    
    return 0;
}