#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000

// removed the printf calls from the original version because
// terminal output takes ~10000 ns and would dominate the measurement
long long multiply_rand(long long min, long long max) {
    long long a = rand() % (max - min + 1) + min;
    long long b = rand() % (max - min + 1) + min;
    return a * b;
}

// ascending order for qsort
int cmp(const void *a, const void *b) {
    double x = *(double *)a, y = *(double *)b;
    return (x > y) - (x < y);
}

int main() {
    srand(42);
    double times[N];

    // measure execution time N times using CLOCK_MONOTONIC
    // this clock never goes backward, ideal for short interval timing
    for (int i = 0; i < N; i++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        multiply_rand(1500000, 2000000);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        // convert to nanoseconds
        times[i] = (t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec);
    }

    // sort so we can read percentiles directly by index
    // times[N-1] is the maximum = empirical WCET
    qsort(times, N, sizeof(double), cmp);

    printf("Statistics over %d runs:\n\n", N);
    printf("  Min        : %.0f ns\n", times[0]);
    printf("  Q1         : %.0f ns\n", times[N / 4]);
    printf("  Q2 median  : %.0f ns\n", times[N / 2]);
    printf("  Q3         : %.0f ns\n", times[3 * N / 4]);
    printf("  Max = WCET : %.0f ns\n", times[N - 1]);

    return 0;
}
