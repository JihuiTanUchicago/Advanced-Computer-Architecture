#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DATA_SIZE (1 << 24)
#define NUM_BINS 256

static inline void generate_data(int* data, int size) {
    srand(42);
    for (int i = 0; i < size; ++i) {
        data[i] = rand() % NUM_BINS;
    }
}

static inline double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}
