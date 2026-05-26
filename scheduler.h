/*
 * scheduler.h  —  Shared types, constants, and declarations
 *
 * Included by every translation unit in the project.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>

/* ── tunables ─────────────────────────────────────────────────── */
#define MAX_PROCS   4096
#define NUM_CORES   2
#define RR_QUANTUM  3
#define LOW_BURST   6     /* burst <= LOW  → Adaptive C0 uses SJF  */
#define MID_BURST   12    /* burst <= MID  → Adaptive C1 uses RR   */
                          /* burst >  MID  → both cores FCFS queue */

#define ALG_FCFS  0
#define ALG_SJF   1
#define ALG_RR    2
#define ALG_ADAP  3
#define NUM_ALGS  4

extern const char *ALG_NAME[NUM_ALGS];

/* ── per-process snapshot ────────────────────────────────────── */
typedef struct {
    int  pid;
    char name[13];   /* 12 visible chars + NUL */
    int  burst;
} Snap;

/* ── per-process result for one algorithm run ────────────────── */
typedef struct {
    int  wt;
    int  tt;
    int  core;           /* -1 = single-core baseline, 0/1 = Adaptive */
    char algo_used;      /* 'F','S','R' */
} Result;

/* ── aggregate metrics for one algorithm ────────────────────── */
typedef struct {
    double avg_wt, avg_tt;
    double throughput;   /* n / avg_tt — differs per algo            */
    double cpu_util;     /* useful_work / (makespan * cores) * 100   */
    long   ctx_sw;
    int    makespan;
    int    cores_used;
} Metrics;

/* ── global poll state (defined in main.c) ───────────────────── */
extern volatile sig_atomic_t g_running;
extern int                   g_poll_no;

/* ── function declarations ───────────────────────────────────── */

/* proc_reader.c */
int  scan_proc(Snap *snaps, int maxn, int *prev_pids, int nprev);

/* schedulers.c */
void     sim_fcfs    (const Snap *s, int n, Result *res);
void     sim_sjf     (const Snap *s, int n, Result *res);
void     sim_rr      (const Snap *s, int n, Result *res);
void     sim_adaptive(const Snap *s, int n, Result *res);

/* metrics.c */
Metrics  compute_metrics(const Snap *s, const Result *res, int n, int aid);

/* display.c */
void     print_all(const Snap *s, Result res[][MAX_PROCS],
                   Metrics *m, int n, int poll_sec);

#endif /* SCHEDULER_H */
