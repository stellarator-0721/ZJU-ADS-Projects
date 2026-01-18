#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include "dijkstra.h"
#include "graph.h"

/*
 * ======================================================================
 * Minimal Binary Heap Implementation
 * ======================================================================
 * This is a standard binary min-heap.
 * It is used *only* by the bidirectional Dijkstra implementation,
 * not by the main unidirectional benchmark functions.
 */

// An item in the binary heap, storing key (priority) and node ID.
typedef struct {
    double key;
    int node;
} HeapItem;

// The binary heap structure.
typedef struct {
    HeapItem *a;    // Array-based storage for the heap
    int cap;        // Capacity of the array
    int size;       // Number of items currently in the heap
} MinHeap;

// Creates a new, empty binary heap with an initial capacity.
static MinHeap* heap_create(int cap) {
    MinHeap* h = malloc(sizeof(MinHeap));
    h->cap = (cap > 0) ? cap : 1024;
    // We allocate cap + 1 because index 0 is unused (1-based indexing).
    h->a = malloc(sizeof(HeapItem) * (h->cap + 1));
    h->size = 0;
    return h;
}

// Frees all memory associated with the binary heap.
static void heap_free(MinHeap* h) {
    free(h->a);
    free(h);
}

// Pushes a new item onto the heap, maintaining the heap property.
// This performs the "sift-up" operation.
static void heap_push(MinHeap* h, double key, int node) {
    // Resize the array if full
    if (h->size + 1 > h->cap) {
        h->cap *= 2;
        h->a = realloc(h->a, sizeof(HeapItem) * (h->cap + 1));
    }
    
    // Start at the new last leaf
    int i = ++h->size;
    
    // Sift up: move parent down while it's larger than the new key
    while (i > 1 && h->a[i/2].key > key) {
        h->a[i] = h->a[i/2];
        i /= 2;
    }
    
    // Place the new item in its correct position
    h->a[i].key = key;
    h->a[i].node = node;
}

// Pops the minimum item (root) from the heap, maintaining the heap property.
// This performs the "sift-down" operation.
static int heap_pop(MinHeap* h, double *key, int *node) {
    if (h->size == 0) return 0; // Heap is empty

    // Get the min item from the root (index 1)
    *key = h->a[1].key;
    *node = h->a[1].node;
    
    // Get the last item to move to the root
    HeapItem last = h->a[h->size--];
    
    int i = 1; // Current index (start at root)
    int c = 2; // Child index (start at left child)
    
    // Sift down
    while (c <= h->size) {
        // Find the smaller of the two children
        if (c + 1 <= h->size && h->a[c+1].key < h->a[c].key) c++;
        
        // If the last item is smaller than or equal to the smallest child,
        // we've found the correct position.
        if (last.key <= h->a[c].key) break;
        
        // Move the smaller child up
        h->a[i] = h->a[c];
        i = c;
        c = 2*i;
    }
    
    // Place the last item in its correct position
    h->a[i] = last;
    return 1;
}

/*
 * ======================================================================
 * Public API Wrappers
 * ======================================================================
 * These are the public functions declared in dijkstra.h.
 * They allocate memory for the distance array and call the
 * internal implementations.
 */

// Internal function prototypes for the actual algorithms
void dijkstra_fib(const Graph *g, int s, double *dist);
void dijkstra_pair(const Graph *g, int s, double *dist);

// Wrapper for Fibonacci heap Dijkstra
double* dijkstra_fibheap(const Graph *g, int s) {
    double* dist = malloc(g->num_nodes * sizeof(double));
    dijkstra_fib(g, s, dist);
    return dist;
}

// Wrapper for Pairing heap Dijkstra
double* dijkstra_pairingheap(const Graph *g, int s) {
    double* dist = malloc(g->num_nodes * sizeof(double));
    dijkstra_pair(g, s, dist);
    return dist;
}

/*
 * ======================================================================
 * Bidirectional Dijkstra (Not part of the main benchmark)
 * ======================================================================
 * This is an alternative implementation for single-pair queries,
 * using two standard binary heaps. It is not called by main.c.
 */

double dijkstra_bi_one_query(const Graph *g, int s, int t, int *out_forward_visited, int *out_backward_visited) {
    if (s == t) {
        if (out_forward_visited) *out_forward_visited = 0;
        if (out_backward_visited) *out_backward_visited = 0;
        return 0.0;
    }

    int n = g->num_nodes;
    
    // distF: distances from source 's'
    // distB: distances from target 't' (on the reverse graph)
    double *distF = malloc(sizeof(double) * n);
    double *distB = malloc(sizeof(double) * n);
    
    // visF/visB: tracks nodes that have been finalized (popped from heap)
    char *visF = malloc(n);
    char *visB = malloc(n);

    for (int i = 0; i < n; i++) {
        distF[i] = DBL_MAX;
        distB[i] = DBL_MAX;
        visF[i] = 0;
        visB[i] = 0;
    }

    // hf: forward heap, hb: backward heap
    MinHeap *hf = heap_create(1024);
    MinHeap *hb = heap_create(1024);

    distF[s] = 0.0;
    distB[t] = 0.0;
    heap_push(hf, 0.0, s);
    heap_push(hb, 0.0, t);

    // best: shortest path length found so far
    double best = DBL_MAX;
    int expandedF = 0, expandedB = 0;

    while (hf->size > 0 || hb->size > 0) {
        
        // --- Forward step ---
        if (hf->size > 0) {
            double key; int u;
            // Pop the minimum valid node from the forward heap
            while (hf->size > 0) {
                if (!heap_pop(hf, &key, &u)) break;
                // Skip "stale" entries (nodes already processed with a shorter path)
                if (key > distF[u]) continue;
                break;
            }

            if (key <= distF[u] && key != DBL_MAX) {
                if (!visF[u]) {
                    visF[u] = 1;
                    expandedF++;
                    // Relax all outgoing edges from 'u'
                    for (Edge *e = g->adj[u]; e; e = e->next) {
                        int v = e->to;
                        double nd = distF[u] + e->weight;
                        if (nd < distF[v]) {
                            distF[v] = nd;
                            heap_push(hf, nd, v);
                        }
                    }
                }
            }
        }

        // --- Backward step ---
        if (hb->size > 0) {
            double key; int u;
            // Pop the minimum valid node from the backward heap
            while (hb->size > 0) {
                if (!heap_pop(hb, &key, &u)) break;
                if (key > distB[u]) continue;
                break;
            }

            if (key <= distB[u] && key != DBL_MAX) {
                if (!visB[u]) {
                    visB[u] = 1;
                    expandedB++;
                    // Relax all *incoming* edges to 'u' (using the reverse graph)
                    for (Edge *e = g->rev_adj[u]; e; e = e->next) {
                        int v = e->to;
                        double nd = distB[u] + e->weight;
                        if (nd < distB[v]) {
                            distB[v] = nd;
                            heap_push(hb, nd, v);
                        }
                    }
                }
            }
        }

        // --- Termination check ---
        // Get the top keys from both heaps
        double topF = (hf->size > 0) ? hf->a[1].key : DBL_MAX;
        double topB = (hb->size > 0) ? hb->a[1].key : DBL_MAX;
        
        // If the sum of the two smallest unvisited nodes is >= the best
        // path found so far, we can stop.
        if (topF + topB >= best) break;

        // --- Update best path ---
        // This is inefficient (O(n)), but simple.
        // A better way is to check only when a node is visited by both searches.
        for (int i = 0; i < n; i++) {
            if (distF[i] < DBL_MAX && distB[i] < DBL_MAX) {
                double cand = distF[i] + distB[i];
                if (cand < best) best = cand;
            }
        }
    }

    // Output stats if requested
    if (out_forward_visited) *out_forward_visited = expandedF;
    if (out_backward_visited) *out_backward_visited = expandedB;

    // Cleanup
    heap_free(hf);
    heap_free(hb);
    free(distF);
    free(distB);
    free(visF);
    free(visB);

    return best;
}

/*
 * ======================================================================
 * Core Unidirectional Dijkstra Implementations
 * ======================================================================
 * These are the actual algorithms used by the main benchmark.
 */

/**
 * Standard Dijkstra's algorithm using a Fibonacci heap.
 * The 'dist' array is pre-allocated by the wrapper.
 */
void dijkstra_fib(const Graph *g, int s, double *dist) {
    // Initialize all distances to infinity
    for (int i = 0; i < g->num_nodes; i++) dist[i] = DBL_MAX;

    FibHeap *H = fib_create(g->num_nodes);
    dist[s] = 0.0;
    fib_insert(H, 0.0, s);

    while (!fib_is_empty(H)) {
        // Get the unvisited node with the smallest distance
        int u = fib_extract_min(H);
        if (u == -1) break; // Heap is empty
        
        // For each neighbor 'v' of 'u'
        for (Edge *e = g->adj[u]; e; e = e->next) {
            int v = e->to;
            double nd = dist[u] + e->weight;
            
            // Relaxation step
            if (nd < dist[v]) {
                dist[v] = nd;
                // Update 'v's priority in the heap (or insert if not present)
                fib_decrease_key(H, v, nd);
            }
        }
    }
    fib_free(H);
}

/**
 * Standard Dijkstra's algorithm using a Pairing heap.
 * The 'dist' array is pre-allocated by the wrapper.
 */
void dijkstra_pair(const Graph *g, int s, double *dist) {
    int n = g->num_nodes;
    for (int i = 0; i < n; i++) dist[i] = DBL_MAX;

    PairingHeap *H = pair_create(n);
    dist[s] = 0.0;
    pair_insert(H, 0.0, s);

    // H->root is the check for "is empty" in this implementation
    while (H->root) {
        // Get the unvisited node with the smallest distance
        int u = pair_extract_min(H);
        if (u == -1) break; // Heap is empty

        // For each neighbor 'v' of 'u'
        for (Edge *e = g->adj[u]; e; e = e->next) {
            int v = e->to;
            double nd = dist[u] + e->weight;
            
            // Relaxation step
            if (nd < dist[v]) {
                dist[v] = nd;
                // Update 'v's priority in the heap (or insert if not present)
                pair_decrease_key(H, v, nd);
            }
        }
    }

    pair_free(H);
}