// pairingheap.h
#ifndef PAIRINGHEAP_H
#define PAIRINGHEAP_H

// Represents a node within the Pairing Heap.
typedef struct PairNode {
    double key;           // Priority (distance)
    int value;            // Node identifier (graph ID)
    struct PairNode *child;   // First child
    struct PairNode *sibling; // Next sibling
    struct PairNode *parent;  // Parent node
    struct PairNode *prev;    // Previous sibling (for efficient cutting)
} PairNode;

// Represents the Pairing Heap.
typedef struct {
    PairNode *root;     // Root of the heap (min element)
    
    // Maps node ID (value) to its PairNode pointer
    // for efficient decrease-key.
    PairNode **map;
    
    int n;              // Max number of nodes (size of the map)
} PairingHeap;

/**
 * Creates a new, empty Pairing heap.
 * 'n' is the max number of nodes, used to pre-allocate the map.
 */
PairingHeap *pair_create(int n);

/**
 * Inserts a new node (key, val) into the heap.
 * If the node already exists, this acts as decrease_key.
 */
void pair_insert(PairingHeap *h, double key, int val);

/**
 * Removes and returns the node with the minimum key.
 * Returns -1 if the heap is empty.
 */
int pair_extract_min(PairingHeap *h);

/**
 * Decreases the key of an existing node.
 * If the node is not in the heap, it will be inserted.
 */
void pair_decrease_key(PairingHeap *h, int val, double newKey);

/**
 * Frees all memory used by the heap.
 */
void pair_free(PairingHeap *h);

#endif // PAIRINGHEAP_H