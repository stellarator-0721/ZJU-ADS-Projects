#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../include/packing.h"

#define RUNS 1000  // Number of repetitions for accurate timing measurements

// Comparator function: sort rectangles by height in descending order
int cmp_height_desc(const void *a, const void *b) {
    Rect *r1 = (Rect *)a;
    Rect *r2 = (Rect *)b;
    return r2->h - r1->h;  // Positive if r2 is taller, negative if r1 is taller
}

// Greedy packing algorithm using Next Fit Decreasing Height (NFDH) strategy
int greedy_pack(Rect *rects, int n, int W) {
    // Step 1: Sort rectangles from tallest to shortest
    qsort(rects, n, sizeof(Rect), cmp_height_desc);

    int total_height = 0;    // Height of all completed shelves
    int shelf_height = 0;    // Height of current shelf (tallest rectangle in it)
    int used_width = 0;      // Width occupied in current shelf

    for (int i = 0; i < n; i++) {
        // Check if rectangle fits in current shelf
        if (used_width + rects[i].w <= W) {
            // Add rectangle to current shelf
            used_width += rects[i].w;
            // Update shelf height if current rectangle is taller
            if (rects[i].h > shelf_height)
                shelf_height = rects[i].h;
        } else {
            // Current shelf is full, start a new one
            total_height += shelf_height;  // Add completed shelf to total
            shelf_height = rects[i].h;     // New shelf starts with current rectangle
            used_width = rects[i].w;       // Reset width counter
        }
    }
    // Add height of the last (possibly incomplete) shelf
    return total_height + shelf_height;
}

// Main function: reads input, runs algorithm, outputs results
int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc < 2) {
        printf("Usage: ./greedy data/input.txt\n");
        return 1;
    }

    // Open input file and read basic parameters
    FILE *fin = fopen(argv[1], "r");
    int n, W;  // n = number of rectangles, W = strip width
    fscanf(fin, "%d %d", &n, &W);

    // Allocate memory and read rectangle data
    Rect *rects = malloc(sizeof(Rect) * n);
    for (int i = 0; i < n; i++)
        fscanf(fin, "%d %d", &rects[i].w, &rects[i].h);
    fclose(fin);

    struct timeval start, end;  // Timing variables
    int height = 0;  // Best height found

    // Execute algorithm multiple times for accurate timing
    gettimeofday(&start, NULL);
    for (int i = 0; i < RUNS; i++) {
        // Create copy to preserve original array order
        Rect *copy = malloc(sizeof(Rect) * n);
        memcpy(copy, rects, sizeof(Rect) * n);
        height = greedy_pack(copy, n, W);
        free(copy);
    }
    gettimeofday(&end, NULL);

    // Calculate execution time in seconds
    double total_time = (end.tv_sec - start.tv_sec) +
                       (end.tv_usec - start.tv_usec) * 1e-6;
    double avg_time = total_time / RUNS;  // Average time per run

    // Generate output filename based on number of rectangles
    char outfile[128];
    snprintf(outfile, sizeof(outfile), "solution/greedy_%d.txt", n);
    FILE *fout = fopen(outfile, "w");

    // Write results to output file
    fprintf(fout, "Algorithm: Greedy NFDH\n");
    fprintf(fout, "Input file: %s\n", argv[1]);
    fprintf(fout, "Rectangles: %d\n", n);
    fprintf(fout, "Strip width: %d\n", W);
    fprintf(fout, "Result height: %d\n", height);
    fprintf(fout, "Runs: %d\n", RUNS);
    fprintf(fout, "Average CPU time (seconds): %.9f\n", avg_time);

    fclose(fout);
    free(rects);
    return 0;
}