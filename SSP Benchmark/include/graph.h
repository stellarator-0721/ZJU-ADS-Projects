#ifndef GRAPH_H
#define GRAPH_H

// Represents a single directed edge in an adjacency list.
typedef struct Edge {
    int to;               // Target node index (0-based)
    double weight;        // Edge weight (distance)
    struct Edge* next;    // Next edge in the list
} Edge;

// Represents the graph using adjacency lists.
typedef struct {
    int num_nodes;        // Total node count
    long num_edges;       // Total edge count
    
    // Forward adjacency lists (adj[i] = list of edges from i)
    Edge** adj;
    
    // Reverse adjacency lists (for bidirectional search)
    // rev_adj[i] = list of edges terminating at i
    Edge** rev_adj;
} Graph;


// Allocates and initializes an empty graph with 'num_nodes' nodes.
Graph* create_graph(int num_nodes);

// Adds a single directed edge (u -> v) to the graph's forward list (g->adj).
void add_edge(Graph* g, int u, int v, double weight);

/**
 * Loads a graph from a DIMACS file.
 * Parses 'p sp' and 'a' lines. Converts 1-based indices to 0-based.
 * Populates *both* the forward (adj) and reverse (rev_adj) lists.
 */
Graph* load_dimacs_graph(const char* filename);

// Frees all memory associated with the graph, including all edges.
void free_graph(Graph* g);

#endif // GRAPH_H