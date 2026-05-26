/*
 * main.c  —  Entry point, signal handling, and poll loop
 *
 * Drives the live comparative scheduler:
 *   1. Parse poll interval from argv
 *   2. Loop: scan /proc → simulate all 4 algorithms → compute metrics → display
 *   3. Exit cleanly on SIGINT / SIGTERM
 */

#include "scheduler.h"

/* ── shared globals (declared extern in scheduler.h) ─────────── */
const char              *ALG_NAME[NUM_ALGS] = {"FCFS", "SJF", "RR(q=3)", "Adaptive"};
volatile sig_atomic_t    g_running          = 1;
int                      g_poll_no          = 0;

static void handle_sig(int s) { (void)s; g_running = 0; }

/* ── main ────────────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    int poll_sec = 2;
    if (argc >= 2) poll_sec = atoi(argv[1]);
    if (poll_sec < 1) poll_sec = 1;

    signal(SIGINT,  handle_sig);
    signal(SIGTERM, handle_sig);

    static Snap   snaps[MAX_PROCS];
    static Result res[NUM_ALGS][MAX_PROCS];
    static int    prev_pids[MAX_PROCS];
    int           nprev = 0;

    printf("Comparative Scheduler starting (poll=%ds)...\n\n", poll_sec);
    sleep(1);

    while (g_running) {
        g_poll_no++;

        int n = scan_proc(snaps, MAX_PROCS, prev_pids, nprev);
        if (n == 0) { sleep(poll_sec); continue; }

        nprev = n < MAX_PROCS ? n : MAX_PROCS;
        for (int i = 0; i < nprev; i++) prev_pids[i] = snaps[i].pid;

        /* Baseline simulations — single-core each */
        sim_fcfs(snaps, n, res[ALG_FCFS]);
        sim_sjf (snaps, n, res[ALG_SJF]);
        sim_rr  (snaps, n, res[ALG_RR]);
        /* Proposed: true dual-core adaptive */
        sim_adaptive(snaps, n, res[ALG_ADAP]);

        Metrics m[NUM_ALGS];
        for (int a = 0; a < NUM_ALGS; a++)
            m[a] = compute_metrics(snaps, res[a], n, a);

        print_all(snaps, res, m, n, poll_sec);

        for (int i = 0; i < poll_sec && g_running; i++) sleep(1);
    }

    printf("\n=== Session ended after %d poll(s). ===\n", g_poll_no);
    return 0;
}
