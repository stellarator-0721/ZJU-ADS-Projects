#ifndef FIBHEAP_H
#define FIBHEAP_H

#include <stdbool.h>

// Opaque type for the Fibonacci Heap.
// The implementation is hidden in the .c file.
typedef struct FibHeap FibHeap;

/**
 * Creates a new Fibonacci heap.
 * 'max_nodes' is used to pre-allocate the node map
 * for efficient decrease-key operations.
 */
FibHeap* fib_create(int max_nodes);

/**
 * Inserts a new node (key, node) into the heap.
 * If the node already exists, this acts as decrease_key.
 */
void fib_insert(FibHeap *H, double key, int node);

/**
 * Removes and returns the node with the minimum key.
 * Returns -1 if the heap is empty.
 */
int fib_extract_min(FibHeap *H);

/**
 * Decreases the key of an existing node.
 * If the node is not in the heap, it will be inserted.
 */
void fib_decrease_key(FibHeap *H, int node, double new_key);

/**
 * Frees all memory used by the heap.
 */
void fib_free(FibHeap *H);

/**
 * Checks if the heap is empty.
 */
bool fib_is_empty(FibHeap *H);

#endif // FIBHEAP_H