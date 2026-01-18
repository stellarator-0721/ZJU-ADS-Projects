#ifndef PACKING_H
#define PACKING_H

// Rectangle structure
typedef struct {
    int w;  // Width
    int h;  // Height
} Rect;

/* Greedy NFDH algorithm */
int greedy_pack(Rect *rects, int n, int W);

/* Simulated Annealing algorithm */
int sa_pack(Rect *rects, int n, int W);

#endif