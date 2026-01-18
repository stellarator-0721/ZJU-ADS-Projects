#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <sys/stat.h>

// Platform-specific includes for directory handling
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#include <dirent.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#include "graph.h"
#include "dijkstra.h"

// Simple cross-platform check for file existence.
int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Simple cross-platform check if a path is a directory.
int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

/**
 * Loads (source, target) query pairs from a file.
 *
 * Reads pairs of integers from 'filename' and stores them in a
 * dynamically allocated array pointed to by 'queries'.
 * Converts the 1-based node indices from the file to 0-based.
 *
 * Returns the total number of query pairs loaded.
 */
int load_query_pairs(const char *filename, int **queries) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    int s, t;
    int count = 0;
    int cap = 1024;
    *queries = malloc(cap * 2 * sizeof(int));

    while (fscanf(fp, "%d %d", &s, &t) == 2) {
        if (count >= cap) {
            // Resize the array if more capacity is needed
            cap *= 2;
            *queries = realloc(*queries, cap * 2 * sizeof(int));
        }
        // Convert from 1-based (file) to 0-based (internal)
        (*queries)[count * 2] = s - 1;
        (*queries)[count * 2 + 1] = t - 1;
        count++;
    }
    fclose(fp);
    return count;
}

/**
 * Runs a single-source Dijkstra from 's' and returns the distance to 't'.
 * This function times the *entire* operation, including heap creation,
 * Dijkstra's algorithm, and distance array cleanup.
 *
 * Returns the shortest distance, or DBL_MAX if unreachable.
 * The time taken is stored in the 'time_used' output parameter.
 */
double run_single_query(const Graph *g, int s, int t, const char *heap_type, double *time_used) {
    clock_t st = clock();
    double result = DBL_MAX;
    
    if (strcmp(heap_type, "fib") == 0) {
        // Run Dijkstra with Fibonacci heap
        double *dist = dijkstra_fibheap(g, s);
        result = dist[t];
        free(dist); // Free the distances after lookup
    } else if (strcmp(heap_type, "pair") == 0) {
        // Run Dijkstra with Pairing heap
        double *dist = dijkstra_pairingheap(g, s);
        result = dist[t];
        free(dist); // Free the distances after lookup
    }
    
    clock_t ed = clock();
    *time_used = (double)(ed - st) / CLOCKS_PER_SEC;
    return result;
}

/**
 * Runs a full test on a query file.
 * Loads all (s, t) pairs from 'query_file', runs run_single_query
 * for each pair, and writes the results to 'output_file'.
 * Prints a summary of the total time and average time per query.
 */
void run_query_test(const Graph *g, const char *query_file, const char *output_file, const char *heap_type) {
    int *queries = NULL;
    int n = load_query_pairs(query_file, &queries);
    if (n == 0) {
        fprintf(stderr, "No queries loaded from %s\n", query_file);
        return;
    }

    FILE *fout = fopen(output_file, "w");
    if (!fout) {
        free(queries);
        return;
    }

    double total_time = 0;
    int reachable = 0;

    // Run and time each query individually
    for (int i = 0; i < n; i++) {
        int s = queries[i * 2];
        int t = queries[i * 2 + 1];

        double query_time;
        double d = run_single_query(g, s, t, heap_type, &query_time);
        total_time += query_time;

        if (d < DBL_MAX) reachable++;

        // Write results to the output file
        fprintf(fout, "%d %d %.6f %.6f\n",
            s + 1, t + 1, d, query_time);
    }

    // Print summary to console
    printf("\n=== Query File Summary ===\n");
    printf("Heap: %s\n", heap_type);
    printf("Queries: %d, Reachable: %d\n", n, reachable);
    printf("Total time: %.6f sec (includes heap build + Dijkstra)\n", total_time);
    printf("Average time per query: %.6f sec\n", total_time / n);

    fclose(fout);
    free(queries);
}

/**
 * Runs a benchmark in "random" mode.
 *
 * This mode is different:
 * 1. It runs *one* full SSSP (Single-Source Shortest Path) calculation
 * from a single random source 's' and times it.
 * 2. It then times 'num' rapid lookups for random targets 't' in the
 * resulting distance array.
 *
 * This separates the one-time SSSP cost from the near-instant
 * cost of array lookups.
 */
void run_random(const Graph *g, int num, int seed, int verbose, const char *heap_type) {
    srand(seed);
    int s = rand() % g->num_nodes;
    printf("Random benchmark mode\n");
    printf("Source = %d\n", s + 1);
    printf("Queries = %d\n", num);

    // 1. Time the full SSSP computation
    clock_t t1 = clock();
    double *dist = NULL;
    if (strcmp(heap_type, "fib") == 0)
        dist = dijkstra_fibheap(g, s);
    else if (strcmp(heap_type, "pair") == 0)
        dist = dijkstra_pairingheap(g, s);
    else {
        fprintf(stderr, "Unknown heap type: %s\n", heap_type);
        return;
    }
    clock_t t2 = clock();
    double preprocess_time = (double)(t2 - t1) / CLOCKS_PER_SEC;
    
    printf("%s heap build + Dijkstra: %.6f sec\n", heap_type, preprocess_time);

    // 2. Time 'num' random lookups in the 'dist' array
    clock_t q1 = clock();
    for (int i = 0; i < num; i++) {
        int t = rand() % g->num_nodes;
        // Use volatile to prevent the compiler from optimizing away the lookup
        volatile double dval = dist[t];
        if (verbose && i < 20) {
            printf("Query %d: t=%d value=%.0f\n", i+1, t+1, dval);
        }
    }
    clock_t q2 = clock();
    double qtime = (double)(q2 - q1) / CLOCKS_PER_SEC;
    
    // Print summary
    printf("\nDistance lookup only time: %.6f sec for %d lookups\n", qtime, num);
    printf("Per lookup time: %.9f sec\n", qtime / num);
    printf("Total time (build + lookups): %.6f sec\n", preprocess_time + qtime);

    free(dist);
}

// Prints the command-line usage instructions.
void usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s <graph_file> <query_file|random|query_dir> <heap_type> [options]\n", prog);
    printf("heap_type: fib | pair\n");
    printf("Examples:\n");
    printf("  %s data/USA-road-d.USA.gr queries/q1.qry fib\n", prog);
    printf("  %s data/USA-road-d.USA.gr random pair 1000 12345 1\n", prog);
    printf("  %s data/USA-road-d.USA.gr query_dir fib\n", prog);
    printf("\nNote: Query time includes heap build + Dijkstra execution time\n");
}

/**
 * Main entry point for the benchmark executable.
 */
int main(int argc, char *argv[]) {
    if (argc < 4) {
        usage(argv[0]);
        return 0;
    }

    const char *graph_file = argv[1];
    const char *heap_type = argv[3];

    if (!file_exists(graph_file)) {
        fprintf(stderr, "Graph file not found\n");
        return -1;
    }

    // Load the graph from file
    Graph *g = load_dimacs_graph(graph_file);
    printf("Graph loaded: %d nodes\n", g->num_nodes);
    printf("==========================================\n");

    const char *q = argv[2]; // The mode/query file argument

    // Mode 1: "random"
    if (strcmp(q, "random") == 0) {
        if (argc < 5) {
            printf("Missing num_queries\n");
            usage(argv[0]);
            free_graph(g);
            return -1;
        }

        int n = atoi(argv[4]);
        int seed = (argc >= 6) ? atoi(argv[5]) : (int)time(NULL);
        int verbose = (argc >= 7) ? atoi(argv[6]) : 0;

        run_random(g, n, seed, verbose, heap_type);
        free_graph(g);
        return 0;
    }

    // Mode 2: Directory
    // If 'q' is a directory, run tests on all .qry files inside it.
    if (is_directory(q)) {
        MKDIR("result"); // Ensure the result directory exists

#ifdef _WIN32
        // Windows directory scanning
        WIN32_FIND_DATA fd;
        char pattern[512];
        sprintf(pattern, "%s\\*.qry", q);
        HANDLE h = FindFirstFile(pattern, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    char full[512], out[512];
                    sprintf(full, "%s\\%s", q, fd.cFileName);
                    sprintf(out, "result\\%s_result.txt", fd.cFileName);
                    run_query_test(g, full, out, heap_type);
                }
            } while (FindNextFile(h, &fd));
            FindClose(h);
        }
#else
        // POSIX directory scanning
        DIR *dir = opendir(q);
        struct dirent *e;
        while ((e = readdir(dir)) != NULL) {
            if (strstr(e->d_name, ".qry")) {
                char full[512], out[512];
                sprintf(full, "%s/%s", q, e->d_name);
                sprintf(out, "result/%s_result.txt", e->d_name);
                run_query_test(g, full, out, heap_type);
            }
        }
        closedir(dir);
#endif
        free_graph(g);
        return 0;
    }

    // Mode 3: Single query file
    if (file_exists(q)) {
        MKDIR("result");
        // Find the base filename (e.g., "q1.qry" from "queries/q1.qry")
        const char *base = strrchr(q, '/');
        if (!base) base = strrchr(q, '\\');
        base = base ? base + 1 : q;

        // Create a corresponding output file path
        char out[256];
        sprintf(out, "result/%s_result.txt", base);
        run_query_test(g, q, out, heap_type);
        free_graph(g);
        return 0;
    }

    // If no mode matched
    fprintf(stderr, "Unknown mode\n");
    usage(argv[0]);
    free_graph(g);
    return -1;
}