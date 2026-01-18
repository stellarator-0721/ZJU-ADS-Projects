#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

/**
 * A static helper function to add an edge to an adjacency list.
 * Allocates a new Edge node and prepends it to the list
 * for the 'from' node. Exits on allocation failure.
 */
static void add_edge_to_list(Edge** adj, int from, int to, double weight) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    if (!e) {
        fprintf(stderr, "Error: failed to allocate edge.\n");
        exit(EXIT_FAILURE);
    }
    e->to = to;
    e->weight = weight;
    e->next = adj[from]; // Prepend to the list
    adj[from] = e;
}

/**
 * Creates and initializes a new Graph structure.
 *
 * Allocates memory for the Graph struct itself and for both the
 * forward (adj) and reverse (rev_adj) adjacency list arrays.
 * Initializes all list heads to NULL using calloc.
 *
 * Exits on allocation failure.
 */
Graph* create_graph(int num_nodes) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (!g) {
        fprintf(stderr, "Error: failed to allocate Graph structure.\n");
        exit(EXIT_FAILURE);
    }

    g->num_nodes = num_nodes;
    g->num_edges = 0;
    
    // Use calloc to initialize all pointers in the adjacency lists to NULL
    g->adj = (Edge**)calloc(num_nodes, sizeof(Edge*));
    g->rev_adj = (Edge**)calloc(num_nodes, sizeof(Edge*));

    if (!g->adj || !g->rev_adj) {
        fprintf(stderr, "Error: failed to allocate adjacency lists.\n");
        free(g->adj);
        free(g->rev_adj);
        free(g);
        exit(EXIT_FAILURE);
    }

    return g;
}

/**
 * Adds a single directed edge (u -> v) to the graph.
 *
 * This function only adds the edge to the forward adjacency list (g->adj).
 * It is primarily used for building the graph internally.
 * Note: It does NOT add the reverse edge to g->rev_adj.
 */
void add_edge(Graph* g, int u, int v, double weight) {
    if (u < 0 || v < 0 || u >= g->num_nodes || v >= g->num_nodes)
        return; // Safety check

    add_edge_to_list(g->adj, u, v, weight);
    g->num_edges++;
}

/**
 * Frees all memory associated with the graph.
 *
 * Iterates through every node and frees the linked lists for
 * both the forward (adj) and reverse (rev_adj) adjacency lists.
 * Then frees the arrays for the lists themselves, and finally
 * frees the Graph struct.
 */
void free_graph(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->num_nodes; i++) {
        // Free the forward list
        Edge* e = g->adj[i];
        while (e) {
            Edge* next = e->next;
            free(e);
            e = next;
        }
        
        // Free the reverse list
        Edge* r = g->rev_adj[i];
        while (r) {
            Edge* next = r->next;
            free(r);
            r = next;
        }
    }
    free(g->adj);
    free(g->rev_adj);
    free(g);
}

/**
 * Loads a graph from a file in the DIMACS 9th Challenge format.
 *
 * This function reads a graph file, parsing 'p sp' (problem) lines
 * to determine graph size, and 'a' (arc) lines to build the graph.
 * It converts the 1-based indexing from the file to the 0-based
 * indexing used by the Graph structure.
 *
 * For each arc, it adds BOTH a forward edge (adj) and a
 * reverse edge (rev_adj) to support bidirectional algorithms.
 */
Graph* load_dimacs_graph(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file %s\n", filename);
        return NULL;
    }

    printf("Reading DIMACS graph file: %s\n", filename);

    int num_nodes = 0, num_edges = 0;
    char line[256];

    // Find the 'p sp <nodes> <edges>' line
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'p') {
            if (sscanf(line, "p sp %d %d", &num_nodes, &num_edges) == 2)
                break;
        }
    }

    if (num_nodes <= 0) {
        fprintf(stderr, "Error: missing 'p sp' header line in %s\n", filename);
        fclose(fp);
        return NULL;
    }

    printf("Allocating graph with %d nodes (≈%d edges expected)...\n", num_nodes, num_edges);

    Graph* g = create_graph(num_nodes);
    if (!g) {
        fclose(fp);
        return NULL;
    }

    long edge_count = 0;
    int from, to;
    double weight;

    // Read all arc ('a') lines
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] != 'a') continue; // Skip non-arc lines

        if (sscanf(line, "a %d %d %lf", &from, &to, &weight) == 3) {
            // Convert from 1-based (DIMACS) to 0-based (internal) indexing
            from -= 1;
            to -= 1;

            if (from >= 0 && from < num_nodes && to >= 0 && to < num_nodes) {
                // Add the forward edge: from -> to
                add_edge_to_list(g->adj, from, to, weight);
                // Add the reverse edge: to -> from
                add_edge_to_list(g->rev_adj, to, from, weight);
                edge_count++;
            }
        }

        // Print progress for very large graphs
        if (edge_count % 5000000 == 0 && edge_count > 0) {
#ifdef _WIN32
            printf("  Loaded %ld edges...\n", edge_count);
#else
            printf("  Loaded %ld edges...\n", edge_count);
#endif
            fflush(stdout);
        }
    }

#ifdef _WIN32
    printf("Finished loading graph: %ld edges read.\n", edge_count);
#else
    printf("Finished loading graph: %ld edges read.\n", edge_count);
#endif

    fclose(fp);
    return g;
}