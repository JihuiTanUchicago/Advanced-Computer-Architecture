#include <omp.h>
#include "histogram_common.h"

void histogram_omp(const int* data, int* bins, int size, int bins_count) {
    #pragma omp parallel
    {
        int* local_bins = calloc(bins_count, sizeof(int));
        #pragma omp for
        for (int i = 0; i < size; ++i) {
            local_bins[data[i]]++;
        }
        #pragma omp critical
        {
            for (int j = 0; j < bins_count; ++j) {
                bins[j] += local_bins[j];
            }
        }
        free(local_bins);
    }
}

int main() {
    int* data = malloc(DATA_SIZE * sizeof(int));
    int* bins = calloc(NUM_BINS, sizeof(int));
    generate_data(data, DATA_SIZE);

    double start = now();
    histogram_omp(data, bins, DATA_SIZE, NUM_BINS);
    double end = now();

    printf("Parallel OMP CPU Runtime: %.6f seconds\n", end - start);

    free(data);
    free(bins);
    return 0;
}
