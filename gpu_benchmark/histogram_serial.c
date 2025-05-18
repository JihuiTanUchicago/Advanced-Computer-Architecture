#include "histogram_common.h"

void histogram_serial(const int* data, int* bins, int size, int bins_count) {
    for (int i = 0; i < size; ++i) {
        bins[data[i]]++;
    }
}

int main() {
    int* data = malloc(DATA_SIZE * sizeof(int));
    int* bins = calloc(NUM_BINS, sizeof(int));
    generate_data(data, DATA_SIZE);

    double start = now();
    histogram_serial(data, bins, DATA_SIZE, NUM_BINS);
    double end = now();

    printf("Serial CPU Runtime: %.6f seconds\n", end - start);

    free(data);
    free(bins);
    return 0;
}
