/*
 * display.c  —  Terminal output and table rendering
 */

#include "scheduler.h"

/* ── table border helper ─────────────────────────────────────── */
static void hrule(const char *l, const char *m, const char *r,
                  const char *dash, const int *w, int ncols) {
    printf("%s", l);
    for (int c = 0; c < ncols; c++) {
        for (int i = 0; i < w[c]; i++) printf("%s", dash);
        printf("%s", c < ncols-1 ? m : r);
    }
    printf("\n");
}

/* ── main display function ───────────────────────────────────── */
void print_all(const Snap *s, Result res[][MAX_PROCS],
               Metrics *m, int n, int poll_sec) {
    printf("\033[2J\033[H");

    /* ── header ──────────────────────────────────────────── */
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║       LIVE COMPARATIVE SCHEDULER  —  Poll #%-4d                 ║\n", g_poll_no);
    printf("║  Processes: %-4d  |  Poll interval: %ds                          ║\n", n, poll_sec);
    printf("║                                                                  ║\n");
    printf("║  Baselines : FCFS / SJF / RR  run on  1 core  (traditional)    ║\n");
    printf("║  Adaptive  : SJF + RR + FCFS  run on  2 cores (proposed)       ║\n");
    printf("║    C0 → short  jobs (burst ≤ %-2d)  via SJF                      ║\n", LOW_BURST);
    printf("║    C1 → medium jobs (burst ≤ %-2d)  via RR(q=%d)                 ║\n", MID_BURST, RR_QUANTUM);
    printf("║    C0+C1 → long jobs (burst > %-2d) via FCFS work-steal          ║\n", MID_BURST);
    printf("╚══════════════════════════════════════════════════════════════════╝\n\n");

    /* ── metrics table ───────────────────────────────────── */
    /* col widths: label(21) + 4 data cols(16 each) */
    static const int mw[] = {21, 16, 16, 16, 16};
    hrule("  ┌", "┬", "┐", "─", mw, 5);
    printf("  │ %-19s │ %14s │ %14s │ %14s │ %14s │\n",
           "Metric",       "FCFS",     "SJF",     "RR(q=3)",  "Adaptive");
    printf("  │ %-19s │ %14s │ %14s │ %14s │ %14s │\n",
           "(cores used)", "(1 core)", "(1 core)", "(1 core)", "(2 cores)");
    hrule("  ├", "┼", "┤", "─", mw, 5);
    printf("  │ %-19s │ %14.2f │ %14.2f │ %14.2f │ %14.2f │\n",
           "Avg Wait (u)",
           m[0].avg_wt, m[1].avg_wt, m[2].avg_wt, m[3].avg_wt);
    printf("  │ %-19s │ %14.2f │ %14.2f │ %14.2f │ %14.2f │\n",
           "Avg Turnaround (u)",
           m[0].avg_tt, m[1].avg_tt, m[2].avg_tt, m[3].avg_tt);
    printf("  │ %-19s │ %14.4f │ %14.4f │ %14.4f │ %14.4f │\n",
           "Throughput(n/avg_tt)",
           m[0].throughput, m[1].throughput, m[2].throughput, m[3].throughput);
    printf("  │ %-19s │ %13.1f%% │ %13.1f%% │ %13.1f%% │ %13.1f%% │\n",
           "CPU Util",
           m[0].cpu_util, m[1].cpu_util, m[2].cpu_util, m[3].cpu_util);
    printf("  │ %-19s │ %14ld │ %14ld │ %14ld │ %14ld │\n",
           "Ctx Switches",
           m[0].ctx_sw, m[1].ctx_sw, m[2].ctx_sw, m[3].ctx_sw);
    printf("  │ %-19s │ %14d │ %14d │ %14d │ %14d │\n",
           "Makespan (u)",
           m[0].makespan, m[1].makespan, m[2].makespan, m[3].makespan);
    hrule("  └", "┴", "┘", "─", mw, 5);
    printf("\n");

    /* ── improvement over best baseline ─────────────────── */
    double best_wt = m[0].avg_wt, best_tt = m[0].avg_tt;
    double best_tp = m[0].throughput;
    int    best_ms = m[0].makespan;
    for (int a = 1; a < NUM_ALGS-1; a++) {
        if (m[a].avg_wt     < best_wt) best_wt = m[a].avg_wt;
        if (m[a].avg_tt     < best_tt) best_tt = m[a].avg_tt;
        if (m[a].throughput > best_tp) best_tp = m[a].throughput;
        if (m[a].makespan   < best_ms) best_ms = m[a].makespan;
    }
    double adwt = m[3].avg_wt, adtt = m[3].avg_tt;
    double adtp = m[3].throughput;
    int    adms = m[3].makespan;

    printf("  ── Adaptive vs Best Baseline ─────────────────────────────────\n");
    printf("  Avg Wait      : %6.2f → %6.2f  (%+.1f%%)\n",
           best_wt, adwt,
           best_wt > 0 ? (adwt - best_wt)*100.0/best_wt : 0);
    printf("  Turnaround    : %6.2f → %6.2f  (%+.1f%%)\n",
           best_tt, adtt,
           best_tt > 0 ? (adtt - best_tt)*100.0/best_tt : 0);
    printf("  Throughput    : %6.4f → %6.4f  (%+.1f%%)\n",
           best_tp, adtp,
           best_tp > 0 ? (adtp - best_tp)*100.0/best_tp : 0);
    printf("  Makespan      : %6d → %6d    (%+.1f%%)\n",
           best_ms, adms,
           best_ms > 0 ? (double)(adms-best_ms)*100.0/best_ms : 0);
    printf("\n");

    /* ── best-per-metric ─────────────────────────────────── */
    int bwt=0, btt=0, btp=0, bcs=0;
    for (int a=1; a<NUM_ALGS; a++) {
        if (m[a].avg_wt     < m[bwt].avg_wt)    bwt=a;
        if (m[a].avg_tt     < m[btt].avg_tt)    btt=a;
        if (m[a].throughput > m[btp].throughput) btp=a;
        if (m[a].ctx_sw     < m[bcs].ctx_sw)    bcs=a;
    }
    printf("  ★ Best Avg Wait    : %-10s  (%.2f)\n",  ALG_NAME[bwt], m[bwt].avg_wt);
    printf("  ★ Best Turnaround  : %-10s  (%.2f)\n",  ALG_NAME[btt], m[btt].avg_tt);
    printf("  ★ Best Throughput  : %-10s  (%.4f)\n",  ALG_NAME[btp], m[btp].throughput);
    printf("  ★ Fewest Ctx-Sw    : %-10s  (%ld)\n\n", ALG_NAME[bcs], m[bcs].ctx_sw);

    /* ── per-process table ───────────────────────────────── */
    /*
     * Column inner widths:
     *  PID(9) Name(14) Burst(7) FCFS(13) SJF(13) RR(13) Adaptive(20)
     */
    static const int pw[] = {9, 14, 7, 13, 13, 13, 20};
    hrule("  ┌", "┬", "┐", "─", pw, 7);
    printf("  │ %-7s │ %-12s │ %-5s │ %-11s │ %-11s │ %-11s │ %-18s │\n",
           "PID", "Name", "Burst", "FCFS", "SJF", "RR(q=3)", "Adaptive");
    printf("  │ %-7s │ %-12s │ %-5s │ %5s %5s │ %5s %5s │ %5s %5s │ %5s %5s %-6s │\n",
           "", "", "", "WT","TT", "WT","TT", "WT","TT", "WT","TT","[Algo]");
    hrule("  ├", "┼", "┤", "─", pw, 7);

    int show = n < 40 ? n : 40;
    for (int i = 0; i < show; i++) {
        const char *sub;
        switch (res[ALG_ADAP][i].algo_used) {
            case 'S': sub = "[SJF ]"; break;
            case 'R': sub = "[RR  ]"; break;
            default:  sub = "[FCFS]"; break;
        }
        char nm[13]; strncpy(nm, s[i].name, 12); nm[12] = '\0';
        printf("  │ %-7d │ %-12s │ %5d │ %5d %5d │ %5d %5d │ %5d %5d │ %5d %5d %s │\n",
               s[i].pid, nm, s[i].burst,
               res[ALG_FCFS][i].wt, res[ALG_FCFS][i].tt,
               res[ALG_SJF ][i].wt, res[ALG_SJF ][i].tt,
               res[ALG_RR  ][i].wt, res[ALG_RR  ][i].tt,
               res[ALG_ADAP][i].wt, res[ALG_ADAP][i].tt, sub);
    }
    if (n > 40)
        printf("  │ %-7s   %-12s   %-5s   %-11s   %-11s   %-11s   %-18s │\n",
               "..","..","..","..","..","..","(40-row limit)");

    hrule("  └", "┴", "┘", "─", pw, 7);
    printf("\n  [Poll #%d — next in %ds — Ctrl+C to stop]\n\n", g_poll_no, poll_sec);
    fflush(stdout);
}
