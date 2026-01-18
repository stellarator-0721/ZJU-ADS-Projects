#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "graph.h"

// Platform-specific includes and definitions for directory creation
#ifdef _WIN32
#include <direct.h>
#define CREATE_DIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define CREATE_DIR(path) mkdir(path, 0755)
#endif

/**
 * Creates a directory if it does not already exist.
 * This is a wrapper for platform-specific mkdir functions.
 */
void create_directory(const char* path) {
#ifdef _WIN32
    _mkdir(path);
#else
    // Set default permissions for new directory on non-Windows
    mkdir(path, 0755);
#endif
}

/**
 * Generates an empty query file.
 * Used for testing the benchmark's behavior with no input.
 */
void generate_empty_queries(const char* folder) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/empty_queries.txt", folder);
    
    FILE* fp = fopen(filename, "w");
    if (fp) {
        // File is created and left empty
        fclose(fp);
        printf("Generated empty queries: %s\n", filename);
    } else {
        printf("Failed to create empty queries file: %s\n", filename);
    }
}

/**
 * Generates a query file for a single-node graph.
 * Writes a single self-loop query (0 0) if the graph has nodes.
 */
void generate_single_node_queries(const char* folder, Graph* g) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/single_node_queries.txt", folder);
    
    FILE* fp = fopen(filename, "w");
    if (fp) {
        if (g->num_nodes >= 1) {
            // A query from the first node (index 0) to itself
            fprintf(fp, "0 0\n");
            printf("Generated single node queries: %s\n", filename);
        } else {
            printf("Graph has no nodes, creating empty single node file\n");
        }
        fclose(fp);
    } else {
        printf("Failed to create single node queries file: %s\n", filename);
    }
}

/**
 * Generates a file with random query pairs (s, t) where s != t.
 * Uses a fixed seed (12345) for reproducible results.
 */
void generate_complete_graph_queries(const char* folder, Graph* g, int count) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/complete_graph_queries_%d.txt", folder, count);
    
    FILE* fp = fopen(filename, "w");
    if (fp) {
        if (g->num_nodes >= 2) {
            // Use a fixed seed for predictable "complete graph" queries
            srand(12345);
            int generated = 0;
            
            printf("Generating complete graph queries...\n");
            while (generated < count) {
                int s = rand() % g->num_nodes;
                int t = rand() % g->num_nodes;
                
                if (s != t) { // Avoid self-loops
                    fprintf(fp, "%d %d\n", s, t);
                    generated++;
                    
                    if (generated % 100 == 0) {
                        printf("  Generated %d/%d complete graph queries...\n", generated, count);
                    }
                }
            }
            printf("Generated complete graph queries: %s (%d pairs)\n", filename, count);
        } else {
            printf("Graph has less than 2 nodes, creating empty complete graph file\n");
        }
        fclose(fp);
    } else {
        printf("Failed to create complete graph queries file: %s\n", filename);
    }
}

/**
 * Generates a file with random query pairs, seeded by current time.
 * This provides a different set of queries on each run.
 */
void generate_normal_queries(const char* folder, Graph* g, int count) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/normal_queries_%d.txt", folder, count);
    
    FILE* fp = fopen(filename, "w");
    if (fp) {
        if (g->num_nodes >= 2) {
            // Use time-based seed for "normal" (non-repeating) queries
            srand(time(NULL));
            int generated = 0;
            int attempts = 0;
            // Set a high attempt limit to avoid infinite loops on sparse graphs
            const int MAX_ATTEMPTS = count * 100;
            
            printf("Generating normal queries...\n");
            
            while (generated < count && attempts < MAX_ATTEMPTS) {
                int s = rand() % g->num_nodes;
                int t = rand() % g->num_nodes;
                attempts++;
                
                // We only require s != t for this test set
                if (s != t) {
                    fprintf(fp, "%d %d\n", s, t);
                    generated++;
                    
                    if (generated % 1000 == 0) {
                        printf("  Generated %d/%d normal queries...\n", generated, count);
                    }
                }
            }
            printf("Generated normal queries: %s (%d pairs, %d attempts)\n", filename, generated, attempts);
            
            if (generated < count) {
                printf("Warning: Only generated %d out of %d queries (max attempts reached)\n", generated, count);
            }
        } else {
            printf("Graph has less than 2 nodes, creating empty normal queries file\n");
        }
        fclose(fp);
    } else {
        printf("Failed to create normal queries file: %s\n", filename);
    }
}

/**
 * Generates a file with a large number of random query pairs.
 * Identical logic to generate_normal_queries, just for a larger 'count'.
 */
void generate_large_scale_queries(const char* folder, Graph* g, int count) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/large_scale_queries_%d.txt", folder, count);
    
    FILE* fp = fopen(filename, "w");
    if (fp) {
        if (g->num_nodes >= 2) {
            srand(time(NULL)); // Use time-based seed
            int generated = 0;
            int attempts = 0;
            const int MAX_ATTEMPTS = count * 100;
            
            printf("Generating large scale queries...\n");
            
            while (generated < count && attempts < MAX_ATTEMPTS) {
                int s = rand() % g->num_nodes;
                int t = rand() % g->num_nodes;
                attempts++;
                
                if (s != t) {
                    fprintf(fp, "%d %d\n", s, t);
                    generated++;
                    
                    if (generated % 1000 == 0) {
                        printf("  Generated %d/%d large scale queries...\n", generated, count);
                    }
                }
            }
            
            printf("Generated large scale queries: %s (%d pairs, %d attempts)\n", filename, generated, attempts);
            
            if (generated < count) {
                printf("Warning: Only generated %d out of %d queries (max attempts reached)\n", generated, count);
            }
        } else {
            printf("Graph has less than 2 nodes, creating empty large scale file\n");
        }
        fclose(fp);
    } else {
        printf("Failed to create large scale queries file: %s\n", filename);
    }
}

/**
 * Generates a small test file (10 queries) using a
 * predefined list of node pairs. This ensures a consistent,
 * small benchmark.
 */
void generate_small_test_queries(const char* folder, Graph* g) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/small_test_queries_10.txt", folder);
    
    FILE* fp = fopen(filename, "w");
    if (fp) {
        printf("Generating small test queries from predefined node pairs...\n");
        
        // A hardcoded list of 20 query pairs (10 pairs, reversed)
        int predefined_pairs[][2] = {
            {1, 2},
            {2, 1},
            {1048577, 1048578},
            {1048578, 1048577},
            {2097153, 3},
            {3, 2097153},
            {4, 1048579},
            {1048579, 4},
            {2097154, 5},
            {5, 2097154},
            {6, 1048580},
            {1048580, 6},
            {1048581, 7},
            {7, 1048581},
            {7, 8},
            {8, 7},
            {1048582, 1048580},
            {1048580, 1048582},
            {7, 1048583},
            {1048583, 7}
        };
        
        int total_pairs = sizeof(predefined_pairs) / sizeof(predefined_pairs[0]);
        int used[20] = {0}; // Track which pairs have been used
        int generated = 0;
        
        srand(time(NULL));
        
        // Randomly select 10 unique pairs from the predefined list
        while (generated < 10 && generated < total_pairs) {
            int idx = rand() % total_pairs;
            if (!used[idx]) {
                int s = predefined_pairs[idx][0];
                int t = predefined_pairs[idx][1];
                
                // Ensure nodes are within the graph's bounds
                if (s >= 0 && s < g->num_nodes && t >= 0 && t < g->num_nodes) {
                    fprintf(fp, "%d %d\n", s, t);
                    used[idx] = 1;
                    generated++;
                    printf("  Generated query %d: %d -> %d\n", generated, s, t);
                }
            }
        }
        
        // If we failed to get 10 (e.g., due to a small graph),
        // fill the rest by iterating sequentially.
        if (generated < 10) {
            printf("Adding more predefined pairs...\n");
            for (int i = 0; i < total_pairs && generated < 10; i++) {
                if (!used[i]) {
                    int s = predefined_pairs[i][0];
                    int t = predefined_pairs[i][1];
                    if (s >= 0 && s < g->num_nodes && t >= 0 && t < g->num_nodes) {
                        fprintf(fp, "%d %d\n", s, t);
                        used[i] = 1;
                        generated++;
                        printf("  Added query %d: %d -> %d\n", generated, s, t);
                    }
                }
            }
        }
        
        printf("Generated small test queries: %s (%d predefined pairs)\n", filename, generated);
        
        fclose(fp);
    } else {
        printf("Failed to create small test queries file: %s\n", filename);
    }
}

/**
 * Generates queries for various graph boundary conditions.
 */
void generate_edge_case_queries(const char* folder, Graph* g) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/edge_case_queries.txt", folder);
    
    FILE* fp = fopen(filename, "w");
    if (fp) {
        if (g->num_nodes >= 1) {
            printf("Generating edge case queries...\n");
            
            // 1. Self-loop
            fprintf(fp, "0 0\n");
            
            if (g->num_nodes >= 2) {
                // 2. Simple path
                fprintf(fp, "0 1\n");
                // 3. Path from max node to min node
                fprintf(fp, "%d %d\n", g->num_nodes-1, 0);
                // 4. Path from min node to max node
                fprintf(fp, "0 %d\n", g->num_nodes-1);
            }
            
            // 5. Find an isolated node (no outgoing edges)
            int isolated_node = -1;
            for (int i = 0; i < g->num_nodes; i++) {
                if (g->adj[i] == NULL) {
                    isolated_node = i;
                    break;
                }
            }
            
            if (isolated_node != -1) {
                fprintf(fp, "%d 0\n", isolated_node);  // Query from isolated node
                fprintf(fp, "0 %d\n", isolated_node);  // Query to isolated node
                printf("Found isolated node: %d\n", isolated_node);
            } else {
                printf("No isolated nodes found\n");
            }
            
            // 6. Find a high-degree node
            int max_degree_node = 0;
            int max_degree = 0;
            for (int i = 0; i < g->num_nodes; i++) {
                if (g->adj[i] != NULL) {
                    int degree = 0;
                    Edge* e = g->adj[i];
                    while (e) {
                        degree++;
                        e = e->next;
                    }
                    if (degree > max_degree) {
                        max_degree = degree;
                        max_degree_node = i;
                    }
                }
            }
            
            if (max_degree > 0) {
                // Query from high-degree node to one of its neighbors
                Edge* e = g->adj[max_degree_node];
                if (e != NULL) {
                    fprintf(fp, "%d %d\n", max_degree_node, e->to);
                }
                printf("Found high-degree node: %d (degree: %d)\n", max_degree_node, max_degree);
            }
            
            printf("Generated edge case queries: %s\n", filename);
        } else {
            printf("Graph has no nodes, creating empty edge case file\n");
        }
        fclose(fp);
    } else {
        printf("Failed to create edge case queries file: %s\n", filename);
    }
}

/**
 * Validates a query file by checking all (s, t) pairs.
 * Ensures that 0 <= s < num_nodes and 0 <= t < num_nodes.
 */
int validate_query_file(const char* folder, const char* filename, Graph* g) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", folder, filename);
    
    FILE* fp = fopen(full_path, "r");
    if (!fp) {
        printf("Cannot open query file: %s\n", full_path);
        return -1;
    }
    
    int valid_pairs = 0;
    int total_pairs = 0;
    int s, t;
    
    // Read all pairs from the file
    while (fscanf(fp, "%d %d", &s, &t) == 2) {
        total_pairs++;
        // Check if nodes are within the valid 0-based index range
        if (s >= 0 && s < g->num_nodes && t >= 0 && t < g->num_nodes) {
            valid_pairs++;
        } else {
            printf("Invalid node indices in query %d: %d -> %d (graph has %d nodes)\n", 
                   total_pairs, s, t, g->num_nodes);
        }
    }
    
    fclose(fp);
    
    printf("Query file %s: %d/%d valid pairs\n", filename, valid_pairs, total_pairs);
    return valid_pairs;
}

/**
 * Main entry point for the query generation executable.
 */
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <graph_file> <queries_folder>\n", argv[0]);
        printf("Example: %s data/USA-road-d.USA.gr Queries\n", argv[0]);
        return 1;
    }
    
    const char* graph_file = argv[1];
    const char* queries_folder = argv[2];
    
    // Create the target directory if it doesn't exist
    create_directory(queries_folder);
    
    printf("Loading graph: %s\n", graph_file);
    // Load the graph only to get its properties (e.g., num_nodes)
    Graph* g = load_dimacs_graph(graph_file);
    if (!g) {
        fprintf(stderr, "Error: failed to load graph file.\n");
        return 1;
    }
    
    printf("Graph loaded successfully: %d nodes\n", g->num_nodes);
    
    // Generate all defined query files
    printf("\nGenerating query files in folder: %s\n", queries_folder);
    printf("==========================================\n");
    
    generate_empty_queries(queries_folder);
    generate_single_node_queries(queries_folder, g);
    generate_small_test_queries(queries_folder, g);
    generate_complete_graph_queries(queries_folder, g, 100);
    generate_normal_queries(queries_folder, g, 1000);
    generate_large_scale_queries(queries_folder, g, 10000);
    generate_edge_case_queries(queries_folder, g);
    
    printf("\n==========================================\n");
    printf("All query files generated successfully!\n");
    
    // Run validation on the generated files
    printf("\nValidating generated query files:\n");
    validate_query_file(queries_folder, "normal_queries_1000.txt", g);
    validate_query_file(queries_folder, "large_scale_queries_10000.txt", g);
    validate_query_file(queries_folder, "small_test_queries_10.txt", g);
    
    free_graph(g);
    return 0;
}