// generate_queries.h
#ifndef GENERATE_QUERIES_H
#define GENERATE_QUERIES_H

#include "graph.h"

// Creates a directory if it does not already exist.
void create_directory(const char* path);

// Generates a query file with no queries.
void generate_empty_queries(const char* folder);

// Generates a query file with a single self-loop query (0 0).
void generate_single_node_queries(const char* folder, Graph* g);

// Generates a file with 'count' random query pairs (fixed seed).
void generate_complete_graph_queries(const char* folder, Graph* g, int count);

// Generates a file with 'count' random query pairs (time seed).
void generate_normal_queries(const char* folder, Graph* g, int count);

// Generates a file with a large number of random query pairs.
void generate_large_scale_queries(const char* folder, Graph* g, int count);

// Generates a small test file (10 queries) from a predefined list.
void generate_small_test_queries(const char* folder, Graph* g);

// Generates queries for various edge cases (self-loops, isolated nodes).
void generate_edge_case_queries(const char* folder, Graph* g);

// Checks if all node indices in a query file are valid.
// Returns the count of valid pairs, or -1 on file error.
int validate_query_file(const char* folder, const char* filename, Graph* g);


// --- Function declarations below are not implemented in the .c file ---

void generate_custom_queries(const char* folder, Graph* g, int count, 
                            const char* filename, unsigned int seed);

void generate_degree_weighted_queries(const char* folder, Graph* g, 
                                     int count, int degree_weighted);

void generate_spatial_queries(const char* folder, Graph* g, int count, 
                             double max_distance);

void analyze_query_file(const char* folder, const char* filename, Graph* g);

#endif /* GENERATE_QUERIES_H */