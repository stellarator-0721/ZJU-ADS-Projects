// recover_design.c
// Backtracking + Constraint Propagation: Solve connector puzzle
// Compile: gcc -O2 -std=c11 -Wall recover_design.c -o recover_design

#include "recover_design.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 55
#define MAXNODE (MAXN * MAXN)
#define MAXSEG (MAXNODE * 2)

typedef struct {
    int r, c, deg;
} Node;

typedef struct {
    int u, v;       // node indices
    int dir;        // 0 horizontal, 1 vertical  
    int len;        // number of intermediate cells
    int cells_r[MAXN * 2];
    int cells_c[MAXN * 2];
} Segment;

// Global variables
int n, m;
int grid[MAXN][MAXN];
Node nodes[MAXNODE];
int node_id[MAXN][MAXN];
int node_cnt;

Segment segs[MAXSEG];
int seg_cnt;

int inc_cnt[MAXNODE];       // number of segments incident to each node
int inc_seg[MAXNODE][5];    // incident segment indices for each node

int seg_val[MAXSEG];        // -1 unknown, 0 not chosen, 1 chosen
int occ[MAXN][MAXN];        // 0 none, 1 horizontal, 2 vertical

int chosen_cnt[MAXNODE];    // number of chosen segments per node
int unknown_cnt_node[MAXNODE]; // number of unknown segments per node

int out_udlr[MAXNODE][4];   // output: up, down, left, right connections

int solved = 0;

/**
 * Add segment between two nodes
 */
void add_segment(int u, int v, int dir, int len, int *cr, int *cc) {
    Segment *s = &segs[seg_cnt];
    s->u = u; s->v = v; s->dir = dir; s->len = len;
    for (int i = 0; i < len; ++i) { 
        s->cells_r[i] = cr[i]; 
        s->cells_c[i] = cc[i]; 
    }
    inc_seg[u][inc_cnt[u]++] = seg_cnt;
    inc_seg[v][inc_cnt[v]++] = seg_cnt;
    seg_cnt++;
}

/**
 * Build all possible segments between adjacent nodes
 */
void build_segments() {
    seg_cnt = 0;
    for (int i = 0; i < node_cnt; ++i) inc_cnt[i] = 0;
    
    // For each node, search for nearest connectors to right and down
    for (int i = 0; i < node_cnt; ++i) {
        int r = nodes[i].r, c = nodes[i].c;
        
        // Search right
        for (int cc = c + 1; cc <= m; ++cc) {
            if (node_id[r][cc] != -1) {
                int j = node_id[r][cc];
                int len = 0;
                int cr[MAXN*2], ccc[MAXN*2];
                for (int x = c + 1; x < cc; ++x) { 
                    cr[len] = r; ccc[len] = x; ++len; 
                }
                add_segment(i, j, 0, len, cr, ccc);
                break;
            }
        }
        
        // Search down  
        for (int rr = r + 1; rr <= n; ++rr) {
            if (node_id[rr][c] != -1) {
                int j = node_id[rr][c];
                int len = 0;
                int cr[MAXN*2], ccc[MAXN*2];
                for (int x = r + 1; x < rr; ++x) { 
                    cr[len] = x; ccc[len] = c; ++len; 
                }
                add_segment(i, j, 1, len, cr, ccc);
                break;
            }
        }
    }
}

/**
 * Initialize solver state
 */
void init_state() {
    for (int i = 0; i < seg_cnt; ++i) seg_val[i] = -1;
    for (int i = 0; i < node_cnt; ++i) {
        chosen_cnt[i] = 0;
        unknown_cnt_node[i] = inc_cnt[i];
    }
    for (int i = 1; i <= n; ++i) 
        for (int j = 1; j <= m; ++j) 
            occ[i][j] = 0;
    solved = 0;
}

/**
 * Check if node configuration is feasible
 */
int node_feasible(int idx) {
    if (chosen_cnt[idx] > nodes[idx].deg) return 0;
    if (chosen_cnt[idx] + unknown_cnt_node[idx] < nodes[idx].deg) return 0;
    return 1;
}

// Structure to track changes for backtracking
typedef struct {
    int seg;
    int prev_val;
    int mod_nodes[2];
    int prev_chosen[2];
    int prev_unknown[2];
    int marked_cells_count;
    int marked_cells_r[MAXN * 2];
    int marked_cells_c[MAXN * 2];
} Change;

/**
 * Apply segment value change with constraint propagation
 */
int apply_segment_set(int sidx, int val, Change *chg) {
    chg->seg = sidx;
    chg->prev_val = seg_val[sidx];
    chg->mod_nodes[0] = segs[sidx].u;
    chg->mod_nodes[1] = segs[sidx].v;
    for (int k = 0; k < 2; ++k) {
        chg->prev_chosen[k] = chosen_cnt[chg->mod_nodes[k]];
        chg->prev_unknown[k] = unknown_cnt_node[chg->mod_nodes[k]];
    }
    chg->marked_cells_count = 0;

    // If segment already has a value, check consistency
    if (seg_val[sidx] != -1) {
        return seg_val[sidx] == val;
    }
    
    seg_val[sidx] = val;
    int u = segs[sidx].u, v = segs[sidx].v;
    unknown_cnt_node[u]--; unknown_cnt_node[v]--;
    
    if (val == 1) {
        chosen_cnt[u]++; chosen_cnt[v]++;
        int dir = segs[sidx].dir;
        
        // Mark occupied cells and check for conflicts
        for (int i = 0; i < segs[sidx].len; ++i) {
            int rr = segs[sidx].cells_r[i], cc = segs[sidx].cells_c[i];
            int need = (dir == 0) ? 1 : 2;  // 1=horizontal, 2=vertical
            
            if (occ[rr][cc] == 0) {
                occ[rr][cc] = need;
                chg->marked_cells_r[chg->marked_cells_count] = rr;
                chg->marked_cells_c[chg->marked_cells_count] = cc;
                chg->marked_cells_count++;
            } else {
                if (occ[rr][cc] != need) return 0; // Conflict detected
            }
        }
    }
    
    return node_feasible(u) && node_feasible(v);
}

/**
 * Undo a change during backtracking
 */
void undo_change(Change *chg) {
    int sidx = chg->seg;
    seg_val[sidx] = chg->prev_val;
    for (int k = 0; k < 2; ++k) {
        int nd = chg->mod_nodes[k];
        chosen_cnt[nd] = chg->prev_chosen[k];
        unknown_cnt_node[nd] = chg->prev_unknown[k];
    }
    for (int i = 0; i < chg->marked_cells_count; ++i) {
        int rr = chg->marked_cells_r[i], cc = chg->marked_cells_c[i];
        occ[rr][cc] = 0;
    }
}

/**
 * Select next node for branching (minimum remaining values heuristic)
 */
int select_node() {
    int best = -1;
    int best_unknown = 1000000;
    for (int i = 0; i < node_cnt; ++i) {
        // Skip satisfied nodes
        if (chosen_cnt[i] == nodes[i].deg && unknown_cnt_node[i] == 0) continue;
        
        // Check for immediate contradiction
        if (!node_feasible(i)) return -2;
        
        // Select node with fewest unknown segments
        int unknown = unknown_cnt_node[i];
        if (unknown < best_unknown) { 
            best = i; 
            best_unknown = unknown; 
        }
    }
    return best;
}

// Forward declaration
void enumerate_combinations_and_branch(int sel, int need, int csz, int *cands);

/**
 * Depth-first search with backtracking
 */
void dfs_solve() {
    if (solved) return;
    
    // Check if solution is complete
    int all_ok = 1;
    for (int i = 0; i < node_cnt; ++i) {
        if (chosen_cnt[i] != nodes[i].deg) { 
            all_ok = 0; 
            break; 
        }
    }
    if (all_ok) {
        // Build output connections
        for (int i = 0; i < node_cnt; ++i) 
            for (int d = 0; d < 4; ++d) 
                out_udlr[i][d] = 0;
                
        for (int s = 0; s < seg_cnt; ++s) {
            if (seg_val[s] == 1) {
                int u = segs[s].u, v = segs[s].v;
                int ru = nodes[u].r, cu = nodes[u].c;
                int rv = nodes[v].r, cv = nodes[v].c;
                
                if (ru == rv) {
                    // Horizontal connection
                    if (cu < cv) { 
                        out_udlr[u][3] = 1; // u right
                        out_udlr[v][2] = 1; // v left
                    } else { 
                        out_udlr[u][2] = 1; // u left  
                        out_udlr[v][3] = 1; // v right
                    }
                } else {
                    // Vertical connection
                    if (ru < rv) { 
                        out_udlr[u][1] = 1; // u down
                        out_udlr[v][0] = 1; // v up
                    } else { 
                        out_udlr[u][0] = 1; // u up
                        out_udlr[v][1] = 1; // v down
                    }
                }
            }
        }
        solved = 1;
        return;
    }

    // Select next node for branching
    int sel = select_node();
    if (sel == -2) return; // Infeasible state
    if (sel == -1) return; // No node selected

    int need = nodes[sel].deg - chosen_cnt[sel];
    int cands[5], csz = 0;
    
    // Collect unknown segments for this node
    for (int k = 0; k < inc_cnt[sel]; ++k) {
        int s = inc_seg[sel][k];
        if (seg_val[s] == -1) cands[csz++] = s;
    }
    
    // Case 1: No more segments needed from this node
    if (need == 0) {
        Change chgs[5];
        int chgcount = 0, ok = 1;
        
        // Set all unknown segments to 0
        for (int i = 0; i < csz; ++i) {
            if (!apply_segment_set(cands[i], 0, &chgs[chgcount++])) { 
                ok = 0; 
                break; 
            }
        }
        if (ok) { 
            dfs_solve(); 
            if (solved) return; 
        }
        // Backtrack
        for (int i = chgcount - 1; i >= 0; --i) 
            undo_change(&chgs[i]);
        return;
    }
    
    // Case 2: Not enough segments available
    if (need > csz) return;
    
    // Case 3: Try all combinations of segments
    enumerate_combinations_and_branch(sel, need, csz, cands);
}

/**
 * Enumerate all combinations of segments for a node
 */
void enumerate_combinations_and_branch(int sel, int need, int csz, int *cands) {
    int total = 1 << csz;
    
    for (int mask = 0; mask < total; ++mask) {
        if (__builtin_popcount((unsigned)mask) != need) continue;
        
        Change chgs[6];
        int chg_count = 0;
        int consistent = 1;
        
        // Set chosen segments to 1
        for (int i = 0; i < csz; ++i) {
            if (mask & (1 << i)) {
                if (!apply_segment_set(cands[i], 1, &chgs[chg_count++])) { 
                    consistent = 0; 
                    break; 
                }
            }
        }
        if (!consistent) { 
            goto backtrack; 
        }
        
        // Set remaining segments to 0
        for (int i = 0; i < csz; ++i) {
            if (!(mask & (1 << i))) {
                if (!apply_segment_set(cands[i], 0, &chgs[chg_count++])) { 
                    consistent = 0; 
                    break; 
                }
            }
        }
        if (!consistent) { 
            goto backtrack; 
        }
        
        // Check feasibility of all affected nodes
        int ok_all = 1;
        for (int k = 0; k < chg_count; ++k) {
            int s = chgs[k].seg;
            int u = segs[s].u, v = segs[s].v;
            if (!node_feasible(u) || !node_feasible(v)) { 
                ok_all = 0; 
                break; 
            }
        }
        
        if (ok_all) {
            dfs_solve();
            if (solved) return;
        }
        
    backtrack:
        for (int k = chg_count - 1; k >= 0; --k) 
            undo_change(&chgs[k]);
        if (solved) return;
    }
}

/**
 * Main function
 */
int main() {
    if (scanf("%d%d", &n, &m) != 2) return 0;
    
    // Read grid and identify nodes
    node_cnt = 0;
    for (int i = 1; i <= n; ++i) 
        for (int j = 1; j <= m; ++j) {
            scanf("%d", &grid[i][j]);
        }
            
    // Initialize node mapping
    for (int i = 1; i <= n; ++i) 
        for (int j = 1; j <= m; ++j) 
            node_id[i][j] = -1;
            
    // Create nodes from grid
    for (int i = 1; i <= n; ++i) 
        for (int j = 1; j <= m; ++j) {
            if (grid[i][j] > 0) {
                nodes[node_cnt].r = i; 
                nodes[node_cnt].c = j; 
                nodes[node_cnt].deg = grid[i][j];
                node_id[i][j] = node_cnt++;
            }
        }
    
    // Validate node degrees
    for (int i = 0; i < node_cnt; ++i) {
        if (nodes[i].deg < 0 || nodes[i].deg > 4) { 
            printf("No Solution\n"); 
            return 0; 
        }
    }
    
    // Build segments and validate
    build_segments();
    for (int i = 0; i < node_cnt; ++i) {
        if (nodes[i].deg > inc_cnt[i]) { 
            printf("No Solution\n"); 
            return 0; 
        }
    }
    
    // Solve the puzzle
    init_state();
    dfs_solve();
    
    if (!solved) { 
        printf("No Solution\n"); 
        return 0; 
    }
    
    // Output solution in row-major order
    for (int i = 1; i <= n; ++i) 
        for (int j = 1; j <= m; ++j) {
            int id = node_id[i][j];
            if (id != -1) {
                printf("%d %d %d %d %d %d\n", i, j, 
                       out_udlr[id][0], out_udlr[id][1], 
                       out_udlr[id][2], out_udlr[id][3]);
            }
        }
        
    return 0;
}