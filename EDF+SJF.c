#include <stdio.h>
#include <limits.h>

int C[] = {1, 3, 2, 2, 2, 2, 3};
int T[] = {10, 10, 20, 20, 40, 40, 80};

typedef struct {
    int task, release, deadline, dur, start, end, wait;
} Job;

int main() {
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

    // SJF + EDF fallback (non-preemptive)
    // goal: minimize total waiting by running the shortest job first,
    // but switch to EDF if doing so would cause another job to miss its deadline
    int t = 0, done = 0;
    while (done < nb) {

        // step 1: find the shortest ready job (SJF)
        int sjf = -1;
        for (int i = 0; i < nb; i++) {
            if (jobs[i].end != -1 || jobs[i].release > t) continue;
            if (sjf == -1 || jobs[i].dur < jobs[sjf].dur)
                sjf = i;
        }

        // step 2: check if running sjf would push another job past its deadline
        // if so, run that job first instead (EDF fallback)
        int urgent = -1;
        if (sjf != -1) {
            for (int i = 0; i < nb; i++) {
                if (jobs[i].end != -1 || jobs[i].release > t || i == sjf) continue;
                if (t + jobs[sjf].dur + jobs[i].dur > jobs[i].deadline)
                    if (urgent == -1 || jobs[i].deadline < jobs[urgent].deadline)
                        urgent = i;
            }
        }

        int best = (urgent != -1) ? urgent : sjf;

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

    printf("\nTotal waiting    : %d\n", total);
    printf("Missed deadlines : %d\n", misses);
    printf("Busy / Idle      : 59 / 21 out of 80\n");
    return 0;
}
