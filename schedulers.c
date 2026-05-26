/*
 * schedulers.c  —  Scheduling algorithm simulations
 *
 * Implements all four algorithms on a process snapshot:
 *
 *   sim_fcfs()     — First Come First Served, single-core
 *   sim_sjf()      — Shortest Job First, single-core
 *   sim_rr()       — Round Robin (quantum=RR_QUANTUM), single-core
 *   sim_adaptive() — Adaptive dual-core (SJF + RR + FCFS work-steal)
 */
 
#include "scheduler.h"


/* FCFS: run in arrival order (snapshot order), sequential */
void sim_fcfs(const Snap *s, int n, Result *res) {
    int clock = 0;
    for (int i = 0; i < n; i++) {
        res[i].wt        = clock;
        res[i].tt        = clock + s[i].burst;
        res[i].core      = -1;
        res[i].algo_used = 'F';
        clock           += s[i].burst;
    }
}

/* SJF: sort by burst ascending, run sequentially */
void sim_sjf(const Snap *s, int n, Result *res) {
    int *ord = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) ord[i] = i;
    /* insertion sort by burst */
    for (int i = 1; i < n; i++) {
        int key = ord[i], j = i - 1;
        while (j >= 0 && s[ord[j]].burst > s[key].burst) {
            ord[j+1] = ord[j]; j--;
        }
        ord[j+1] = key;
    }
    int clock = 0;
    for (int ii = 0; ii < n; ii++) {
        int i = ord[ii];
        res[i].wt        = clock;
        res[i].tt        = clock + s[i].burst;
        res[i].core      = -1;
        res[i].algo_used = 'S';
        clock           += s[i].burst;
    }
    free(ord);
}

/* RR: round-robin with fixed quantum, single queue */
void sim_rr(const Snap *s, int n, Result *res) {
    int *rem  = malloc(n * sizeof(int));
    int *done = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) { rem[i] = s[i].burst; done[i] = 0; }
    int clock = 0, finished = 0, idx = 0;
    while (finished < n) {
        int ran = 0;
        for (int i = 0; i < n; i++) {
            int j = (idx + i) % n;
            if (rem[j] == 0) continue;
            int sl = (rem[j] < RR_QUANTUM) ? rem[j] : RR_QUANTUM;
            clock += sl; rem[j] -= sl;
            if (rem[j] == 0) { done[j] = clock; finished++; }
            idx = (j + 1) % n;
            ran = 1; break;
        }
        if (!ran) break;
    }
    for (int i = 0; i < n; i++) {
        res[i].tt        = done[i];
        res[i].wt        = done[i] - s[i].burst;
        if (res[i].wt < 0) res[i].wt = 0;
        res[i].core      = -1;
        res[i].algo_used = 'R';
    }
    free(rem); free(done);
}


/* Classify a burst into its adaptive sub-algorithm */
static char adap_sub(int burst) {
    if (burst <= LOW_BURST) return 'S';
    if (burst <= MID_BURST) return 'R';
    return 'F';
}

void sim_adaptive(const Snap *s, int n, Result *res) {
    int *si = malloc(n * sizeof(int)), ns = 0;   /* short  → C0 SJF         */
    int *ri = malloc(n * sizeof(int)), nr = 0;   /* medium → C1 RR          */
    int *fi = malloc(n * sizeof(int)), nf = 0;   /* long   → work-steal FCFS */

    /* Bucket each process by burst size */
    for (int i = 0; i < n; i++) {
        char a = adap_sub(s[i].burst);
        if      (a == 'S') si[ns++] = i;
        else if (a == 'R') ri[nr++] = i;
        else               fi[nf++] = i;
    }

    /* Sort short bucket by burst (SJF order) */
    for (int i = 1; i < ns; i++) {
        int key = si[i], j = i - 1;
        while (j >= 0 && s[si[j]].burst > s[key].burst) {
            si[j+1] = si[j]; j--;
        }
        si[j+1] = key;
    }

    int clk0 = 0;   /* Core 0 clock — runs independently */
    int clk1 = 0;   /* Core 1 clock — runs independently */

    /* ── Core 0: all short jobs in SJF order ── */
    for (int ii = 0; ii < ns; ii++) {
        int i = si[ii];
        res[i].wt        = clk0;
        res[i].tt        = clk0 + s[i].burst;
        res[i].core      = 0;
        res[i].algo_used = 'S';
        clk0            += s[i].burst;
    }

    /* ── Core 1: all medium jobs in RR order ── */
    if (nr > 0) {
        int *rem  = malloc(nr * sizeof(int));
        int *done = malloc(nr * sizeof(int));
        for (int i = 0; i < nr; i++) { rem[i] = s[ri[i]].burst; done[i] = 0; }
        int clock = clk1, finished = 0, idx = 0;
        while (finished < nr) {
            int ran = 0;
            for (int i = 0; i < nr; i++) {
                int j = (idx + i) % nr;
                if (rem[j] == 0) continue;
                int sl = (rem[j] < RR_QUANTUM) ? rem[j] : RR_QUANTUM;
                clock += sl; rem[j] -= sl;
                if (rem[j] == 0) { done[j] = clock; finished++; }
                idx = (j + 1) % nr;
                ran = 1; break;
            }
            if (!ran) break;
        }
        clk1 = clock;
        for (int ii = 0; ii < nr; ii++) {
            int i = ri[ii];
            res[i].tt        = done[ii];
            res[i].wt        = done[ii] - s[i].burst;
            if (res[i].wt < 0) res[i].wt = 0;
            res[i].core      = 1;
            res[i].algo_used = 'R';
        }
        free(rem); free(done);
    }

    /* ── Long jobs: work-steal — pick whichever core is free first ── */
    for (int ii = 0; ii < nf; ii++) {
        int i   = fi[ii];
        int c   = (clk0 <= clk1) ? 0 : 1;
        int *ck = (c == 0) ? &clk0 : &clk1;
        res[i].wt        = *ck;
        res[i].tt        = *ck + s[i].burst;
        res[i].core      = c;
        res[i].algo_used = 'F';
        *ck             += s[i].burst;
    }

    free(si); free(ri); free(fi);
}
