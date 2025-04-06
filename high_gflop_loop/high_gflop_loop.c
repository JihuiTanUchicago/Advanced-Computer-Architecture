#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N (100000000)
#define FLOPS_PER_ITERATION 10

int main() {
    double x = 1.101, y = -1.100;
    double acc = 0.0;

    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        // 10 FLOPS per iteration
        acc += (x + y);
        acc += (x + y);
        acc += (x + y);
        acc += (x + y);
        acc += (x + y);
    }
    clock_t end = clock();

    double time = (double)(end - start) / CLOCKS_PER_SEC;
    double gflops = (N * FLOPS_PER_ITERATION) / (time * 1e9);

    printf("Time: %f s\n", time);
    printf("Performance: %f GFLOP/s\n", gflops);
    printf("Expected: %f \n", 500000.0);
    printf("Got: %f \n", acc);

    return 0;
}
