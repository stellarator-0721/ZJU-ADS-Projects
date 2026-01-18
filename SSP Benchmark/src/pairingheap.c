#include <stdlib.h>
#include <stdio.h>
#include "pairingheap.h"

// --- Static Helper Function Prototypes ---

// Recursively frees a node and its entire subtree
static void free_pair_node(PairNode *node);
// Merges two heap trees, returning the new root
static PairNode *pair_merge(PairNode *a, PairNode *b);
// Combines a list of sibling nodes using a multi-pass strategy
static PairNode *pair_combine(PairNode *first);


// --- Helper Function Implementations ---

/**
 * Merges two heap trees (roots 'a' and 'b').
 * The tree with the smaller root key becomes the parent.
 * The tree with the larger key is added as the first child
 * of the smaller-key tree.
 * Returns the root of the merged tree.
 */
static PairNode *pair_merge(PairNode *a, PairNode *b) {
    if (!a) return b;
    if (!b) return a;

    // Ensure 'a' is the tree with the smaller key
    if (b->key < a->key) { 
        PairNode *t = a; 
        a = b; 
        b = t; 
    }
    
    // Link 'b' as the first child of 'a'
    b->sibling = a->child;
    if (b->sibling) {
        b->sibling->prev = b; // Update prev pointer of old first child
    }
    a->child = b;
    b->parent = a;
    b->prev = NULL; // 'b' is now the first child, so it has no prev sibling
    
    return a;
}

/**
 * Combines a list of sibling nodes (linked via 'sibling' pointer).
 * This is the core of the extract_min operation.
 * It uses a multi-pass strategy:
 * 1. First pass: Iterates left-to-right, merging pairs of trees.
 * 2. Subsequent passes: Merges the resulting trees from right-to-left
 * until only one tree remains.
 */
static PairNode *pair_combine(PairNode *first) {
    if (!first) return NULL;

    // Use a dynamic array to store the merged sub-heaps from the first pass
    int cap = 128, n = 0;
    PairNode **arr = malloc(cap * sizeof(PairNode*));
    if (!arr) return NULL; // Allocation failure

    PairNode *current = first;
    
    // --- First Pass: Left-to-right pairwise merging ---
    while (current) {
        PairNode *a = current;
        PairNode *b = NULL;
        current = current->sibling; // Move to next
        a->sibling = NULL; // Disconnect 'a'

        if (current) {
            b = current; // Get the second tree in the pair
            current = current->sibling; // Move to the *next* pair
            b->sibling = NULL; // Disconnect 'b'
        }

        // Resize array if needed
        if (n >= cap) {
            cap *= 2;
            arr = realloc(arr, cap * sizeof(PairNode*));
            if (!arr) return NULL; // Realloc failure
        }
        
        // Add the merged pair (a, b) to the array
        arr[n++] = pair_merge(a, b);
    }

    // --- Second Pass: Right-to-left merge ---
    // Merge all trees in the array into a single tree
    // (This implementation is slightly different, it's a multi-pass reduction)
    while (n > 1) {
        int new_n = 0;
        // Merge pairs in the array
        for (int i = 0; i < n; i += 2) {
            if (i + 1 < n) {
                // Merge two trees
                arr[new_n++] = pair_merge(arr[i], arr[i+1]);
            } else {
                // Odd one out, just carry it over
                arr[new_n++] = arr[i];
            }
        }
        n = new_n; // Set new array size
    }

    PairNode *result = (n > 0) ? arr[0] : NULL;
    free(arr);
    return result;
}

/**
 * Recursively frees a node, its children, and its siblings.
 */
static void free_pair_node(PairNode *node) {
    if (!node) return;
    
    // We must traverse recursively, not just follow siblings,
    // as free(node) invalidates access to node->sibling.
    PairNode *child = node->child;
    PairNode *sibling = node->sibling;
    
    free(node); // Free current node first
    
    // Then free the sub-lists
    if (child) free_pair_node(child);
    if (sibling) free_pair_node(sibling);
}


// --- Public API Functions ---

PairingHeap *pair_create(int n) {
    PairingHeap *h = malloc(sizeof(PairingHeap));
    if (!h) return NULL;
    
    h->root = NULL;
    h->n = n;
    
    // Allocate the map for O(1) node access
    h->map = calloc(n, sizeof(PairNode *));
    if (!h->map) {
        free(h);
        return NULL;
    }
    return h;
}

void pair_insert(PairingHeap *h, double key, int val) {
    if (val < 0 || val >= h->n) return; // Safety check

    // If node already exists, just decrease its key
    if (h->map[val]) {
        pair_decrease_key(h, val, key);
        return;
    }

    // Create the new node
    PairNode *node = malloc(sizeof(PairNode));
    if (!node) return; // Allocation failure
    
    node->key = key;
    node->value = val;
    node->child = NULL;
    node->sibling = NULL;
    node->parent = NULL;
    node->prev = NULL;

    // Merge the new node with the root
    h->root = pair_merge(h->root, node);
    
    // Store in map
    h->map[val] = node;
}

int pair_extract_min(PairingHeap *h) {
    if (!h->root) return -1; // Heap is empty

    int v = h->root->value;
    h->map[v] = NULL; // Remove from map

    PairNode *old_root = h->root;
    PairNode *first_child = h->root->child;
    
    // Disconnect all children from the old root
    PairNode *child = first_child;
    while (child) {
        child->parent = NULL;
        child = child->sibling;
    }

    // Rebuild the heap by merging all children
    h->root = pair_combine(first_child);
    
    free(old_root);
    return v;
}

void pair_decrease_key(PairingHeap *h, int val, double newKey) {
    if (val < 0 || val >= h->n) return; // Safety check
    
    PairNode *x = h->map[val];
    if (!x) {
        // Node is not in the heap, so insert it.
        pair_insert(h, newKey, val);
        return;
    }

    if (newKey >= x->key) return; // Not a valid decrease-key
    
    x->key = newKey;

    if (x == h->root) return; // Root's key, just update it

    PairNode *p = x->parent;
    if (!p) return; // Should not happen if x != h->root, but good safety check

    // Cut 'x' from its parent 'p' and its siblings
    if (p->child == x) {
        // x is the first child
        p->child = x->sibling;
        if (x->sibling) {
            x->sibling->prev = NULL;
        }
    } else {
        // x is a middle or last child
        if (x->prev) {
            x->prev->sibling = x->sibling;
        }
        if (x->sibling) {
            x->sibling->prev = x->prev;
        }
    }

    // Reset x's pointers and merge it with the root
    x->parent = NULL;
    x->sibling = NULL;
    x->prev = NULL;
    h->root = pair_merge(h->root, x);
}

void pair_free(PairingHeap *h) {
    if (!h) return;
    
    // Recursively free all nodes
    if (h->root) free_pair_node(h->root);
    
    // Free the map and the heap structure
    free(h->map);
    free(h);
}