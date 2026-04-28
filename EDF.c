#include <stdio.h>
#include <limits.h>

// WCET and period for each task t1..t7
// implicit deadline: each job must finish before the next one starts
int C[] = {1, 3, 2, 2, 2, 2, 3};
int T[] = {10, 10, 20, 20, 40, 40, 80};

typedef struct {
    int task;      // index of the parent task (0..6)
    int release;   // time when the job becomes available
    int deadline;  // absolute deadline
    int dur;       // execution time
    int start;     // when it started (-1 = not yet)
    int end;       // when it finished (-1 = not yet)
    int wait;      // start - release
} Job;

int main() {
    // schedulability check: total utilisation must be <= 1
    double U = 0;
    for (int i = 0; i < 7; i++)
        U += (double)C[i] / T[i];
    printf("U = %.4f -> %s\n\n", U, U <= 1.0 ? "schedulable" : "not schedulable");

    // generate all jobs over one hyperperiod H = LCM(10,10,20,20,40,40,80) = 80
    
    Job jobs[50];
    int nb = 0;
    for (int i = 0; i < 7; i++) {
        for (int k = 0; k < 80 / T[i]; k++) {
            jobs[nb].task     = i;
            jobs[nb].release  = k * T[i];
            jobs[nb].deadline = (k + 1) * T[i];
            jobs[nb].dur      = C[i];
            jobs[nb].start    = -1;
            jobs[nb].end      = -1;
            nb++;
        }
    }

    // EDF non-preemptive: at each idle moment, pick the ready job
    // with the earliest (smallest) absolute deadline
    int t = 0, done = 0;
    while (done < nb) {

        int best = -1;
        for (int i = 0; i < nb; i++) {
            if (jobs[i].end != -1 || jobs[i].release > t) continue;
            if (best == -1 || jobs[i].deadline < jobs[best].deadline)
                best = i;
        }

        // no job is ready: jump directly to the next release time
        if (best == -1) {
            int nxt = INT_MAX;
            for (int i = 0; i < nb; i++)
                if (jobs[i].end == -1 && jobs[i].release > t && jobs[i].release < nxt)
                    nxt = jobs[i].release;
            t = nxt;
            continue;
        }

        // run to completion (non-preemptive)
        jobs[best].start = t;
        t += jobs[best].dur;
        jobs[best].end = t;
        done++;
    }

    // print results in chronological order (sorted by start time)
    int total = 0, misses = 0;
    printf("%-8s | %-5s | %-5s | %-6s | %-6s | %-7s | %s\n",
           "Job", "Rel", "DL", "Start", "End", "Waiting", "Status");
    printf("---------|-------|-------|--------|--------|---------|--------\n");

    int seen[50] = {0};
    for (int o = 0; o < nb; o++) {
        int nx = -1;
        for (int i = 0; i < nb; i++)
            if (!seen[i] && (nx == -1 || jobs[i].start < jobs[nx].start))
                nx = i;
        seen[nx] = 1;
        jobs[nx].wait = jobs[nx].start - jobs[nx].release;
        total += jobs[nx].wait;
        int miss = jobs[nx].end > jobs[nx].deadline;
        if (miss) misses++;
        printf("t%d[%d]    | %-5d | %-5d | %-6d | %-6d | %-7d | %s\n",
               jobs[nx].task + 1,
               jobs[nx].release / T[jobs[nx].task],
               jobs[nx].release, jobs[nx].deadline,
               jobs[nx].start, jobs[nx].end,
               jobs[nx].wait, miss ? "MISS" : "ok");
    }

    // busy = sum(C[i] * H/T[i]) = 59, idle = 80 - 59 = 21 (fixed regardless of algorithm)
    printf("\nTotal waiting    : %d\n", total);
    printf("Missed deadlines : %d\n", misses);
    printf("Busy / Idle      : 59 / 21 out of 80\n");
    return 0;
}
