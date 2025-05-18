#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <algorithm>
#include <chrono>

#define DATA_SIZE (1 << 24)
#define NUM_BINS 256

__global__ void histogram_fine_gpu(const int* data, int* bins, int size) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if (i < size) {
        atomicAdd(&bins[data[i]], 1);
    }
}

void generate_data(int* data) {
    srand(42);
    for (int i = 0; i < DATA_SIZE; ++i) {
        data[i] = rand() % NUM_BINS;
    }
}

int main() {
    int* data_h = (int*)malloc(DATA_SIZE * sizeof(int));
    generate_data(data_h);

    int* data_d; cudaMalloc(&data_d, DATA_SIZE * sizeof(int));
    int* bins_d; cudaMalloc(&bins_d, NUM_BINS * sizeof(int));
    cudaMemcpy(data_d, data_h, DATA_SIZE * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemset(bins_d, 0, NUM_BINS * sizeof(int));

    int blockSize = 1024;
    int numBlocks = (DATA_SIZE + blockSize - 1) / blockSize;

    auto start = std::chrono::high_resolution_clock::now();
    histogram_fine_gpu<<<numBlocks, blockSize>>>(data_d, bins_d, DATA_SIZE);
    cudaDeviceSynchronize();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    printf("GPU Fine Runtime: %.6f seconds\n", elapsed.count());

    cudaFree(data_d);
    cudaFree(bins_d);
    free(data_h);
    return 0;
}
