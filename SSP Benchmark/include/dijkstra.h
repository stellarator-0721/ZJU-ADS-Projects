#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include "fibheap.h"
#include "pairingheap.h"

/**
 * Runs Dijkstra's algorithm using a Fibonacci heap.
 * Returns a new array with distances from the source 's'.
 * Caller must free the returned array.
 */
double* dijkstra_fibheap(const Graph *g, int s);

/**
 * Runs Dijkstra's algorithm using a Pairing heap.
 * Returns a new array with distances from the source 's'.
 * Caller must free the returned array.
 */
double* dijkstra_pairingheap(const Graph *g, int s);

#endif // DIJKSTRA_H