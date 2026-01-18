#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define MAXN 10005  // Maximum number of rectangles allowed
#define RUNS 10     // Number of independent SA runs to perform

typedef struct {
    int w, h;  // Width and height of a rectangle
} Rect;

Rect rects[MAXN];      // Global array storing all rectangles
int base_order[MAXN];  // Initial order (indices) for SA
int order[MAXN];       // Current order being evaluated
int n, W;              // Number of rectangles and strip width

// Comparator for sorting indices by rectangle height (descending)
int cmp_height_desc(const void *a, const void *b) {
    int i = *(int *)a;  // Dereference to get rectangle index
    int j = *(int *)b;
    return rects[j].h - rects[i].h;  // Compare heights
}

// Compute packing height for a given rectangle order
int compute_height(int ord[]) {
    int cur_w = 0, cur_h = 0, total_h = 0;
    
    for (int i = 0; i < n; i++) {
        Rect r = rects[ord[i]];  // Get rectangle at position i in order
        
        if (cur_w + r.w <= W) {
            // Rectangle fits in current shelf
            cur_w += r.w;
            if (r.h > cur_h) cur_h = r.h;  // Update shelf height
        } else {
            // Start new shelf
            total_h += cur_h;  // Add completed shelf to total
            cur_w = r.w;       // Reset for new shelf
            cur_h = r.h;
        }
    }
    return total_h + cur_h;  // Add last shelf's height
}

// Neighborhood operator 1: Randomly swap two rectangles
void random_swap(int ord[]) {
    int i = rand() % n;
    int j = rand() % n;
    int t = ord[i];
    ord[i] = ord[j];
    ord[j] = t;
}

// Neighborhood operator 2: Shuffle a random contiguous segment
void segment_shuffle(int ord[]) {
    int l = rand() % n;
    int r = rand() % n;
    if (l > r) { int t = l; l = r; r = t; }  // Ensure l <= r
    
    // Randomly shuffle elements within [l, r]
    for (int i = l; i <= r; i++) {
        int j = l + rand() % (r - l + 1);
        int t = ord[i];
        ord[i] = ord[j];
        ord[j] = t;
    }
}

int main(int argc, char *argv[]) {
    // Validate command line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: ./sa data/input_xxx.txt\n");
        return EXIT_FAILURE;
    }

    // Open input file
    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    // Read number of rectangles and strip width
    if (fscanf(fin, "%d %d", &n, &W) != 2) {
        fprintf(stderr, "Error: Failed to read n and W.\n");
        fclose(fin);
        return EXIT_FAILURE;
    }

    // Validate problem size
    if (n <= 0 || n > MAXN) {
        fprintf(stderr, "Error: n (%d) out of range (1 ~ %d).\n", n, MAXN);
        fclose(fin);
        return EXIT_FAILURE;
    }

    // Read rectangle data and validate
    for (int i = 0; i < n; i++) {
        if (fscanf(fin, "%d %d", &rects[i].w, &rects[i].h) != 2) {
            fprintf(stderr, "Error: Failed to read rectangle %d.\n", i);
            fclose(fin);
            return EXIT_FAILURE;
        }
        if (rects[i].w <= 0 || rects[i].h <= 0) {
            fprintf(stderr, "Error: Invalid rectangle size at %d.\n", i);
            fclose(fin);
            return EXIT_FAILURE;
        }
        if (rects[i].w > W) {
            fprintf(stderr, "Error: Rectangle %d too wide.\n", i);
            fclose(fin);
            return EXIT_FAILURE;
        }
        base_order[i] = i;  // Initialize order as [0, 1, 2, ..., n-1]
    }
    fclose(fin);

    srand((unsigned)time(NULL));  // Seed random number generator

    // Generate initial greedy solution
    qsort(base_order, n, sizeof(int), cmp_height_desc);
    int greedy_h = compute_height(base_order);

    double total_time = 0.0;
    int best_height_global = greedy_h;  // Best height across all runs

    // Perform multiple independent SA runs
    for (int run = 0; run < RUNS; run++) {
        // Start from greedy solution
        memcpy(order, base_order, sizeof(int) * n);
        int cur_h = greedy_h;
        int best_h = greedy_h;  // Best height in this run

        // SA parameters
        double T = 0.1 * greedy_h;  // Initial temperature
        double Tmin = 1e-4;         // Minimum temperature
        double alpha = (n >= 1000) ? 0.999 : 0.995;  // Cooling rate

        struct timeval start, end;
        gettimeofday(&start, NULL);

        // Simulated Annealing main loop
        while (T > Tmin) {
            int old_h = cur_h;

            // Apply neighborhood operator (20% chance for major perturbation)
            if (rand() % 5 == 0)
                segment_shuffle(order);  // Major move
            else
                random_swap(order);      // Minor move

            cur_h = compute_height(order);
            int delta = cur_h - old_h;  // Change in objective

            // Metropolis acceptance criterion
            if (delta < 0) {
                // Always accept improving moves
                if (cur_h < best_h) best_h = cur_h;
            } else if (exp(-delta / T) > (double)rand() / RAND_MAX) {
                // Accept worse move with probability exp(-Δ/T)
                if (cur_h < best_h) best_h = cur_h;
            } else {
                // Reject move: restore previous state
                cur_h = old_h;
            }

            T *= alpha;  // Cool down temperature
        }

        gettimeofday(&end, NULL);
        double run_time = (end.tv_sec - start.tv_sec) + 
                         (end.tv_usec - start.tv_usec) * 1e-6;
        
        total_time += run_time;
        if (best_h < best_height_global)
            best_height_global = best_h;  // Update global best
    }

    double avg_time = total_time / RUNS;

    // Write results to output file
    char outfile[128];
    snprintf(outfile, sizeof(outfile), "solution/sa_%d.txt", n);
    FILE *fout = fopen(outfile, "w");
    if (!fout) {
        perror("Error opening output file");
        return EXIT_FAILURE;
    }

    fprintf(fout, "Algorithm: Simulated Annealing\n");
    fprintf(fout, "Input file: %s\n", argv[1]);
    fprintf(fout, "Rectangles: %d\n", n);
    fprintf(fout, "Strip width: %d\n", W);
    fprintf(fout, "Result height: %d\n", best_height_global);
    fprintf(fout, "Runs: %d\n", RUNS);
    fprintf(fout, "Average CPU time (seconds): %.9f\n", avg_time);

    fclose(fout);
    return EXIT_SUCCESS;
}