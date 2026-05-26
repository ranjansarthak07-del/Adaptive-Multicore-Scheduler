/*
 * metrics.c  —  Aggregate metrics computation
 */

#include "scheduler.h"

Metrics compute_metrics(const Snap *s, const Result *res, int n, int aid) {
    Metrics m = {0};
    double sw = 0, st = 0;
    int ms = 0;
    long total_burst = 0;

    for (int i = 0; i < n; i++) {
        sw += res[i].wt;
        st += res[i].tt;
        if (res[i].tt > ms) ms = res[i].tt;
        total_burst += s[i].burst;
    }

    m.avg_wt     = n ? sw / n : 0;
    m.avg_tt     = n ? st / n : 0;
    m.makespan   = ms;
    m.cores_used = (aid == ALG_ADAP) ? NUM_CORES : 1;


    m.throughput = (m.avg_tt > 0) ? (double)n / m.avg_tt : 0;

    /* CPU utilisation: useful ticks / total available core-ticks */
    long avail = (long)ms * m.cores_used;
    m.cpu_util = avail > 0 ? (double)total_burst * 100.0 / avail : 0;

    /* Context switch estimates */
    if (aid == ALG_FCFS || aid == ALG_SJF) {
        m.ctx_sw = n;   /* one switch per process */
    } else if (aid == ALG_RR) {
        long c = 0;
        for (int i = 0; i < n; i++)
            c += (s[i].burst + RR_QUANTUM - 1) / RR_QUANTUM;
        m.ctx_sw = c;
    } else { /* Adaptive */
        long c = 0;
        for (int i = 0; i < n; i++)
            c += (res[i].algo_used == 'R')
                 ? (s[i].burst + RR_QUANTUM - 1) / RR_QUANTUM : 1;
        m.ctx_sw = c;
    }
    return m;
}
