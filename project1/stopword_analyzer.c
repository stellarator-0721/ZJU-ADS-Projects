#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_WORD_LENGTH 100
#define MAX_WORDS 1000
#define MAX_STOPWORDS 2000

// 单词结构
typedef struct {
    char word[MAX_WORD_LENGTH];
    int frequency;
    double percentage;
    int rank;
} WordInfo;

// 函数声明
int read_enhanced_frequency_file(const char* filename, WordInfo words[], int max_words);
int is_likely_stopword(const char* word, int frequency, double percentage, int rank);
void save_stopwords(const char* filename, WordInfo words[], int count);
void display_statistics(WordInfo words[], int total_words, int stopword_count);
void analyze_word_patterns(WordInfo words[], int count);

int main() {
    const char* frequency_file = "C:\\Users\\ccy\\Desktop\\project1\\word_frequency_enhenced_results.txt";
    const char* output_file = "enhanced_stopwords_analysis.txt";
    const char* pure_output_file = "enhanced_stopwords_pure.txt";
    
    WordInfo words[MAX_WORDS];
    int word_count;
    
    printf("Enhanced Stopword Analyzer for Shakespeare Texts\n");
    printf("================================================\n");
    printf("Reading from: %s\n", frequency_file);
    
    // 读取增强过滤后的词频文件
    word_count = read_enhanced_frequency_file(frequency_file, words, MAX_WORDS);
    if (word_count == 0) {
        printf("Error: Could not read frequency file or no words found.\n");
        printf("Please check if the file exists and has the correct format.\n");
        return 1;
    }
    
    printf("Successfully loaded %d words from enhanced frequency file.\n", word_count);
    
    // 分析并识别可能的停用词
    WordInfo potential_stopwords[MAX_WORDS];
    int stopword_count = 0;
    
    for (int i = 0; i < word_count; i++) {
        if (is_likely_stopword(words[i].word, words[i].frequency, words[i].percentage, words[i].rank)) {
            potential_stopwords[stopword_count] = words[i];
            stopword_count++;
        }
    }
    
    // 显示统计信息
    display_statistics(words, word_count, stopword_count);
    
    // 分析单词模式
    analyze_word_patterns(potential_stopwords, stopword_count);
    
    // 保存停用词到文件
    save_stopwords(output_file, potential_stopwords, stopword_count);
    
    // 保存纯单词列表
    FILE* pure_file = fopen(pure_output_file, "w");
    if (pure_file) {
        for (int i = 0; i < stopword_count; i++) {
            fprintf(pure_file, "%s\n", potential_stopwords[i].word);
        }
        fclose(pure_file);
        printf("Pure word list saved to: %s\n", pure_output_file);
    }
    
    printf("\nAnalysis completed. Results saved to:\n");
    printf("- %s (detailed analysis)\n", output_file);
    printf("- %s (pure word list)\n", pure_output_file);
    
    return 0;
}

// 读取增强过滤后的词频文件
int read_enhanced_frequency_file(const char* filename, WordInfo words[], int max_words) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return 0;
    }
    
    char line[512];
    int count = 0;
    bool in_data_section = false;
    bool found_header = false;
    
    printf("Parsing frequency file...\n");
    
    while (fgets(line, sizeof(line), file) && count < max_words) {
        // 移除换行符
        line[strcspn(line, "\n")] = 0;
        
        // 寻找数据部分的开始
        if (strstr(line, "------------------------------------------------") != NULL) {
            in_data_section = true;
            found_header = true;
            continue;
        }
        
        // 如果找到数据部分并且行不为空
        if (in_data_section && strlen(line) > 10) {
            // 尝试解析格式： "1      to                   20152      2.3743%"
            int rank;
            char word[MAX_WORD_LENGTH];
            int frequency;
            double percentage;
            
            // 更灵活的解析方法
            char* tokens[4];
            char* token = strtok(line, " ");
            int token_count = 0;
            
            while (token != NULL && token_count < 4) {
                tokens[token_count++] = token;
                token = strtok(NULL, " ");
            }
            
            if (token_count >= 4) {
                // 解析排名
                rank = atoi(tokens[0]);
                
                // 解析单词（可能是第二个或第三个token）
                strcpy(word, tokens[1]);
                
                // 如果第三个token不是数字，可能是单词的一部分（处理带空格的单词）
                if (token_count > 2 && !isdigit(tokens[2][0])) {
                    strcat(word, " ");
                    strcat(word, tokens[2]);
                    frequency = atoi(tokens[3]);
                    if (token_count > 4) {
                        percentage = atof(tokens[4]);
                    } else {
                        percentage = 0.0;
                    }
                } else {
                    frequency = atoi(tokens[2]);
                    percentage = atof(tokens[3]);
                }
                
                // 移除百分比符号
                char* percent_sign = strchr(tokens[token_count-1], '%');
                if (percent_sign) *percent_sign = '\0';
                
                if (strlen(word) > 0 && rank > 0) {
                    strcpy(words[count].word, word);
                    words[count].frequency = frequency;
                    words[count].percentage = percentage;
                    words[count].rank = rank;
                    count++;
                    
                    if (count <= 5) {
                        printf("  Parsed: rank=%d, word='%s', freq=%d, perc=%.4f%%\n", 
                               rank, word, frequency, percentage);
                    }
                }
            }
        }
        
        // 如果已经解析了一些数据但遇到空行或新章节，停止解析
        if (in_data_section && count > 0 && 
            (strlen(line) == 0 || strstr(line, "===") != NULL)) {
            break;
        }
    }
    
    fclose(file);
    
    if (!found_header) {
        printf("Warning: Could not find data section header in the file.\n");
    }
    
    printf("Successfully parsed %d words.\n", count);
    return count;
}

// 判断单词是否可能是停用词（增强版规则）
int is_likely_stopword(const char* word, int frequency, double percentage, int rank) {
    // 规则1: 高排名且高频率的单词
    if (rank <= 30 && percentage > 0.08) {
        return 1;
    }
    
    // 规则2: 短单词 (长度<=4) 且频率较高
    if (strlen(word) <= 4 && percentage > 0.05) {
        return 1;
    }
    
    // 规则3: 非常短的单词 (长度<=2) 自动考虑
    if (strlen(word) <= 2 && percentage > 0.01) {
        return 1;
    }
    
    // 规则4: 常见功能词和代词
    const char* common_functional_words[] = {
        // 代词和限定词
        "i", "you", "he", "she", "it", "we", "they", "me", "him", "her", 
        "us", "them", "my", "your", "his", "its", "our", "their", "mine", "yours",
        "this", "that", "these", "those",
        
        // 介词
        "to", "with", "upon", "within", "without", "under", "over", "between",
        "among", "through", "during", "before", "after",
        
        // 连词
        "and", "but", "or", "nor", "for", "yet", "so", "though", "although",
        
        // 助动词和情态动词
        "will", "would", "shall", "should", "can", "could", "may", "might", "must",
        
        // 常见动词
        "have", "has", "had", "do", "does", "did", "say", "said", "see", "saw",
        "come", "came", "go", "went", "know", "knew", "think", "thought",
        
        // 其他常见功能词
        "very", "more", "most", "some", "any", "all", "such", "only", "just",
        
        NULL
    };
    
    for (int i = 0; common_functional_words[i] != NULL; i++) {
        if (strcmp(word, common_functional_words[i]) == 0) {
            return 1;
        }
    }
    
    // 规则5: 莎士比亚特有的高频功能词
    const char* shakespeare_specific[] = {
        "thou", "thy", "thee", "thine", "hath", "doth", "hast", "art", 
        "'tis", "exeunt", "enter", "pray", "nay", "aye", "forsooth",
        "wherefore", "hark", "behold", "alas", "adieu", "oft", "ere",
        NULL
    };
    
    for (int i = 0; shakespeare_specific[i] != NULL; i++) {
        if (strcmp(word, shakespeare_specific[i]) == 0) {
            return 1;
        }
    }
    
    // 规则6: 包含撇号的单词
    if (strchr(word, '\'') != NULL && percentage > 0.03) {
        return 1;
    }
    
    // 规则7: 高频抽象概念词（在文学中常见但信息量有限）
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

// 保存停用词到文件
void save_stopwords(const char* filename, WordInfo words[], int count) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot create output file: %s\n", filename);
        return;
    }
    
    fprintf(file, "# Enhanced Stopword Analysis for Shakespeare Texts\n");
    fprintf(file, "# Generated from: word_frequency_enhanced_filtering.txt\n");
    fprintf(file, "# Total suggested stopwords: %d\n", count);
    fprintf(file, "# \n");
    fprintf(file, "# Format: word (frequency, percentage, rank)\n");
    fprintf(file, "# Criteria: High frequency, short words, functional words, Shakespeare-specific terms\n");
    fprintf(file, "# \n");
    
    // 按类别分组显示
    fprintf(file, "# === HIGH FREQUENCY FUNCTIONAL WORDS ===\n");
    for (int i = 0; i < count; i++) {
        if (words[i].rank <= 20 && words[i].percentage > 0.1) {
            fprintf(file, "%-15s # freq: %6d, %6.4f%%, rank: %2d (HIGH)\n", 
                    words[i].word, words[i].frequency, words[i].percentage, words[i].rank);
        }
    }
    
    fprintf(file, "\n# === MEDIUM FREQUENCY WORDS ===\n");
    for (int i = 0; i < count; i++) {
        if (words[i].rank > 20 && words[i].rank <= 50) {
            fprintf(file, "%-15s # freq: %6d, %6.4f%%, rank: %2d\n", 
                    words[i].word, words[i].frequency, words[i].percentage, words[i].rank);
        }
    }
    
    fprintf(file, "\n# === SHAKESPEARE-SPECIFIC TERMS ===\n");
    const char* shakespeare_terms[] = {"thou", "thy", "thee", "hath", "doth", "'tis", "exeunt", NULL};
    for (int i = 0; i < count; i++) {
        for (int j = 0; shakespeare_terms[j] != NULL; j++) {
            if (strcmp(words[i].word, shakespeare_terms[j]) == 0) {
                fprintf(file, "%-15s # freq: %6d, %6.4f%%, rank: %2d (SHAKESPEARE)\n", 
                        words[i].word, words[i].frequency, words[i].percentage, words[i].rank);
                break;
            }
        }
    }
    
    fclose(file);
}

// 显示统计信息
void display_statistics(WordInfo words[], int total_words, int stopword_count) {
    printf("\n=== Enhanced Stopword Analysis Results ===\n");
    printf("Total words analyzed: %d\n", total_words);
    printf("Potential stopwords identified: %d\n", stopword_count);
    printf("Percentage of words flagged as potential stopwords: %.2f%%\n", 
           (double)stopword_count / total_words * 100);
    
    if (stopword_count > 0) {
        printf("\nTop 25 potential stopwords:\n");
        printf("%-4s %-15s %-10s %-12s %s\n", "Rank", "Word", "Frequency", "Percentage", "Category");
        printf("----------------------------------------------------------\n");
        
        int display_count = (stopword_count < 25) ? stopword_count : 25;
        for (int i = 0; i < display_count; i++) {
            const char* category = "MEDIUM";
            if (words[i].rank <= 10) category = "HIGH";
            else if (words[i].rank <= 20) category = "MED-HIGH";
            
            printf("%-4d %-15s %-10d %-11.4f%% %s\n", 
                   words[i].rank, words[i].word, words[i].frequency, 
                   words[i].percentage, category);
        }
        
        // 计算过滤效果
        int total_filtered_frequency = 0;
        int total_all_frequency = 0;
        
        for (int i = 0; i < stopword_count; i++) {
            total_filtered_frequency += words[i].frequency;
        }
        for (int i = 0; i < total_words; i++) {
            total_all_frequency += words[i].frequency;
        }
        
        if (total_all_frequency > 0) {
            double reduction_percentage = (double)total_filtered_frequency / total_all_frequency * 100;
            printf("\nFiltering Statistics:\n");
            printf("Total frequency of identified stopwords: %d\n", total_filtered_frequency);
            printf("Estimated total frequency: %d\n", total_all_frequency);
            printf("Filtering would reduce word count by: %.2f%%\n", reduction_percentage);
        }
    }
}

// 分析单词模式
void analyze_word_patterns(WordInfo words[], int count) {
    printf("\n=== Word Pattern Analysis ===\n");
    
    int short_words = 0;      // 长度 <= 3
    int medium_words = 0;     // 长度 4-5
    int functional_words = 0; // 明显功能词
    int shakespeare_words = 0; // 莎士比亚特有
    
    const char* shakespeare_terms[] = {"thou", "thy", "thee", "hath", "doth", "'tis", "exeunt", NULL};
    
    for (int i = 0; i < count; i++) {
        int len = strlen(words[i].word);
        
        if (len <= 3) short_words++;
        else if (len <= 5) medium_words++;
        
        // 检查是否是莎士比亚特有词汇
        for (int j = 0; shakespeare_terms[j] != NULL; j++) {
            if (strcmp(words[i].word, shakespeare_terms[j]) == 0) {
                shakespeare_words++;
                break;
            }
        }
    }
    
    printf("Short words (<=3 chars): %d (%.1f%%)\n", short_words, (double)short_words/count*100);
    printf("Medium words (4-5 chars): %d (%.1f%%)\n", medium_words, (double)medium_words/count*100);
    printf("Long words (>5 chars): %d (%.1f%%)\n", count - short_words - medium_words, 
           (double)(count - short_words - medium_words)/count*100);
    printf("Shakespeare-specific terms: %d (%.1f%%)\n", shakespeare_words, 
           (double)shakespeare_words/count*100);
}