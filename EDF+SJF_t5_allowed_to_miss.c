#include <stdio.h>
#include <limits.h>

int C[] = {1, 3, 2, 2, 2, 2, 3};
int T[] = {10, 10, 20, 20, 40, 40, 80};

typedef struct {
    int task, release, deadline, dur, start, end, wait;
    int bg;  // background flag: 1 for tau5 (allowed to miss its deadline)
} Job;

int main() {
    // tau5 is flagged as background so it can be deprioritised
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
            jobs[nb].bg       = (i == 4);
            nb++;
        }
    }

    // same SJF + EDF fallback as part 1, but tau5 is treated as a background task:
    // excluded from both the SJF selection and the deadline safety check,
    // it only runs when no critical job is ready
    int t = 0, done = 0;
    while (done < nb) {

        // step 1: shortest ready job, ignoring tau5
        int sjf = -1;
        for (int i = 0; i < nb; i++) {
            if (jobs[i].end != -1 || jobs[i].release > t || jobs[i].bg) continue;
            if (sjf == -1 || jobs[i].dur < jobs[sjf].dur)
                sjf = i;
        }

        // step 2: EDF fallback if a critical job would miss its deadline
        int urgent = -1;
        if (sjf != -1) {
            for (int i = 0; i < nb; i++) {
                if (jobs[i].end != -1 || jobs[i].release > t || jobs[i].bg || i == sjf) continue;
                if (t + jobs[sjf].dur + jobs[i].dur > jobs[i].deadline)
                    if (urgent == -1 || jobs[i].deadline < jobs[urgent].deadline)
                        urgent = i;
            }
        }

        int best = (urgent != -1) ? urgent : sjf;

        // step 3: no critical job ready, use tau5 as filler
        if (best == -1) {
            for (int i = 0; i < nb; i++) {
                if (jobs[i].end == -1 && jobs[i].release <= t && jobs[i].bg)
                    { best = i; break; }
            }
        }

        if (best == -1) {
            int nxt = INT_MAX;
            for (int i = 0; i < nb; i++)
                if (jobs[i].end == -1 && jobs[i].release > t && jobs[i].release < nxt)
                    nxt = jobs[i].release;
            t = nxt;
            continue;
        }

        jobs[best].start = t;
        t += jobs[best].dur;
        jobs[best].end = t;
        done++;
    }

    int total = 0, misses = 0, t5_miss = 0;
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
        if (miss && !jobs[nx].bg) misses++;
        if (miss &&  jobs[nx].bg) t5_miss++;
        printf("t%d[%d]    | %-5d | %-5d | %-6d | %-6d | %-7d | %s\n",
               jobs[nx].task + 1,
               jobs[nx].release / T[jobs[nx].task],
               jobs[nx].release, jobs[nx].deadline,
               jobs[nx].start, jobs[nx].end,
               jobs[nx].wait,
               miss ? (jobs[nx].bg ? "MISS tau5" : "MISS") : "ok");
    }

    printf("\nTotal waiting    : %d\n", total);
    printf("Missed deadlines : %d (excluding tau5)\n", misses);
    printf("tau5 misses      : %d\n", t5_miss);
    printf("Busy / Idle      : 59 / 21 out of 80\n");
    return 0;
}
