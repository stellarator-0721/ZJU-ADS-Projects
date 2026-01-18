#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdbool.h>
#include <math.h>
#include "fibheap.h"

// Internal representation of a node in the Fibonacci heap
typedef struct FibNode {
    int node;           // The node identifier (e.g., graph node ID)
    double key;         // The priority (e.g., distance)
    int degree;         // Number of children in the child list
    bool mark;          // Mark flag (for cascading cuts)
    struct FibNode *parent;    // Pointer to the parent node
    struct FibNode *child;     // Pointer to the first child
    struct FibNode *left;      // Left sibling in a circular doubly-linked list
    struct FibNode *right;     // Right sibling in a circular doubly-linked list
} FibNode;

// Internal representation of the Fibonacci heap
struct FibHeap {
    FibNode *min;       // Pointer to the node with the minimum key (root list)
    int n;              // Number of nodes currently in the heap
    int max_nodes;      // Maximum node ID (size of the map array)
    
    /**
     * Maps a node ID (int) to its corresponding FibNode pointer.
     * This is essential for an O(1) lookup time during decrease_key.
     */
    FibNode **map;
};

// --- Static Helper Function Prototypes ---

// Allocates and initializes a new heap node
static FibNode* fib_new_node(int node, double key);
// Links node y as a child of node x
static void fib_link(FibHeap *H, FibNode *y, FibNode *x);
// Consolidates the root list to ensure unique tree degrees
static void fib_consolidate(FibHeap *H);
// Cuts node x from its parent y and moves it to the root list
static void fib_cut(FibHeap *H, FibNode *x, FibNode *y);
// Performs a cascading cut, moving up from a node
static void fib_cascading_cut(FibHeap *H, FibNode *y);
// Recursively frees a node and its entire subtree
static void fib_free_node(FibNode *node);


// --- Public API Functions ---

FibHeap* fib_create(int max_nodes) {
    FibHeap *H = (FibHeap*)malloc(sizeof(FibHeap));
    if (!H) return NULL;
    
    H->min = NULL;
    H->n = 0;
    H->max_nodes = max_nodes;
    
    // Allocate the map for O(1) node access
    H->map = (FibNode**)calloc(max_nodes, sizeof(FibNode*));
    if (!H->map) {
        free(H);
        return NULL;
    }
    return H;
}

void fib_insert(FibHeap *H, double key, int node) {
    if (node < 0 || node >= H->max_nodes) {
        // Safety check
        printf("Error: node index %d out of bounds (max: %d)\n", node, H->max_nodes);
        return;
    }
    
    // If node already exists, just decrease its key
    if (H->map[node] != NULL) {
        fib_decrease_key(H, node, key);
        return;
    }
    
    // Create a new node and add it to the map
    FibNode *x = fib_new_node(node, key);
    if (!x) return; // Allocation failure
    H->map[node] = x;
    
    if (H->min == NULL) {
        // This is the first node in the heap
        H->min = x;
    } else {
        // Add the new node to the root list
        x->left = H->min;
        x->right = H->min->right;
        H->min->right->left = x;
        H->min->right = x;
        
        // Update the min pointer if this new node is smaller
        if (key < H->min->key) {
            H->min = x;
        }
    }
    H->n++;
}

int fib_extract_min(FibHeap *H) {
    FibNode *z = H->min;
    if (z == NULL) return -1; // Heap is empty
    
    // Promote all children of the min node to the root list
    if (z->child != NULL) {
        FibNode *child = z->child;
        FibNode *next_child;
        do {
            next_child = child->right;
            
            // Add child to the root list
            child->left = H->min;
            child->right = H->min->right;
            H->min->right->left = child;
            H->min->right = child;
            
            child->parent = NULL;
            child = next_child;
        } while (child != z->child); // Iterate through all children
        z->child = NULL;
    }
    
    // Remove z from the root list
    if (z->right == z) {
        // z was the only node in the root list
        H->min = NULL;
    } else {
        z->left->right = z->right;
        z->right->left = z->left;
        H->min = z->right; // Set temporary min
        
        // Consolidate the heap
        fib_consolidate(H);
    }
    
    int result = z->node;
    H->map[result] = NULL; // Remove from map
    free(z);
    H->n--;
    
    return result;
}

void fib_decrease_key(FibHeap *H, int node, double new_key) {
    if (node < 0 || node >= H->max_nodes) {
        printf("Error: node index %d out of bounds (max: %d)\n", node, H->max_nodes);
        return;
    }
    
    FibNode *x = H->map[node];
    if (x == NULL) {
        // Node is not in the heap, so insert it.
        // This is common in Dijkstra's when visiting a node for the first time.
        fib_insert(H, new_key, node);
        return;
    }
    
    if (new_key > x->key) {
        // New key is larger, this is not a valid decrease-key operation
        return;
    }
    
    x->key = new_key;
    FibNode *y = x->parent;
    
    // If the heap property is now violated (child < parent)
    if (y != NULL && x->key < y->key) {
        fib_cut(H, x, y);
        fib_cascading_cut(H, y);
    }
    
    // Update the heap's minimum pointer if necessary
    if (H->min != NULL && x->key < H->min->key) {
        H->min = x;
    }
}

bool fib_is_empty(FibHeap *H) {
    return H->min == NULL;
}

void fib_free(FibHeap *H) {
    if (H == NULL) return;
    
    // Free all nodes in the heap
    if (H->min != NULL) {
        fib_free_node(H->min);
    }
    
    // Free the map and the heap structure itself
    free(H->map);
    free(H);
}


// --- Static Helper Function Implementations ---

static FibNode* fib_new_node(int node, double key) {
    FibNode *x = (FibNode*)malloc(sizeof(FibNode));
    if (!x) return NULL;
    
    x->node = node;
    x->key = key;
    x->degree = 0;
    x->mark = false;
    x->parent = NULL;
    x->child = NULL;
    
    // Make it a circular list of one
    x->left = x;
    x->right = x;
    return x;
}

static void fib_link(FibHeap *H, FibNode *y, FibNode *x) {
    // 1. Remove y from the root list
    y->left->right = y->right;
    y->right->left = y->left;
    
    // 2. Make y a child of x
    y->parent = x;
    if (x->child == NULL) {
        // x has no children, y becomes the only child
        x->child = y;
        y->left = y->right = y;
    } else {
        // Add y to x's child list (which is also a circular list)
        y->left = x->child;
        y->right = x->child->right;
        x->child->right->left = y;
        x->child->right = y;
    }
    
    // 3. Update x's degree
    x->degree++;
    
    // 4. Unmark y (it just became a child)
    y->mark = false;
}

static void fib_consolidate(FibHeap *H) {
    if (H->min == NULL) return;
    
    // Max degree is bounded by O(log n)
    // We add 2 for safety (e.g., if n=1, log(1)=0)
    int max_degree = (int)(log(H->n) / log(2)) + 2;
    
    // A is an array of pointers to trees, indexed by degree
    FibNode **A = (FibNode**)calloc(max_degree + 1, sizeof(FibNode*));
    if (!A) return; // Allocation failure
    
    // Count nodes in the root list to avoid infinite loops
    // in case of list corruption.
    FibNode *w = H->min;
    int root_count = 0;
    FibNode *current = w;
    do {
        root_count++;
        current = current->right;
    } while (current != w);
    
    // Iterate through all nodes in the root list
    for (int i = 0; i < root_count; i++) {
        FibNode *x = w;
        w = w->right; // Move to next before processing x
        
        int d = x->degree;
        
        // Merge trees of the same degree
        while (A[d] != NULL) {
            FibNode *y = A[d]; // Another tree with the same degree
            
            // Ensure x is always the tree with the smaller key
            if (x->key > y->key) {
                FibNode *temp = x;
                x = y;
                y = temp;
            }
            
            // Link y (larger key) under x (smaller key)
            fib_link(H, y, x);
            
            // Clear the slot and move to the next degree
            A[d] = NULL;
            d++;
        }
        // Store the (potentially new) tree x in its degree slot
        A[d] = x;
    }
    
    // Rebuild the root list from the consolidated trees in A
    H->min = NULL;
    for (int i = 0; i <= max_degree; i++) {
        if (A[i] != NULL) {
            if (H->min == NULL) {
                // First tree in the new root list
                H->min = A[i];
                A[i]->left = A[i]->right = A[i];
            } else {
                // Add A[i] to the root list
                A[i]->left = H->min;
                A[i]->right = H->min->right;
                H->min->right->left = A[i];
                H->min->right = A[i];
                
                // Update min if needed
                if (A[i]->key < H->min->key) {
                    H->min = A[i];
                }
            }
        }
    }
    
    free(A);
}

static void fib_cut(FibHeap *H, FibNode *x, FibNode *y) {
    // 1. Remove x from y's child list
    if (x->right == x) {
        // x was the only child
        y->child = NULL;
    } else {
        // x has siblings
        if (y->child == x) {
            y->child = x->right; // Update parent's child pointer
        }
        x->left->right = x->right;
        x->right->left = x->left;
    }
    y->degree--;
    
    // 2. Add x to the root list
    x->left = H->min;
    x->right = H->min->right;
    H->min->right->left = x;
    H->min->right = x;
    
    // 3. Update x's fields
    x->parent = NULL;
    x->mark = false; // Nodes in the root list are never marked
    
    // 4. Update heap's min pointer if necessary
    if (x->key < H->min->key) {
        H->min = x;
    }
}

static void fib_cascading_cut(FibHeap *H, FibNode *y) {
    FibNode *z = y->parent;
    if (z != NULL) {
        // If y is not marked, mark it (it lost one child)
        if (!y->mark) {
            y->mark = true;
        } else {
            // If y is marked, it has now lost a second child.
            // Cut y from its parent z and move it to the root list.
            fib_cut(H, y, z);
            // Recurse up the tree
            fib_cascading_cut(H, z);
        }
    }
}

static void fib_free_node(FibNode *node) {
    if (node == NULL) return;
    
    // Iterate through the circular list and free all nodes
    FibNode *current = node;
    FibNode *start = node;
    do {
        FibNode *next = current->right;
        
        // Recursively free the child list
        if (current->child != NULL) {
            fib_free_node(current->child);
        }
        
        free(current);
        current = next;
    } while (current != start); // Stop when we've looped back
}