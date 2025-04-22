#include <iostream>
#include <getopt.h>
#include <vector>
#include <cstring>

#include "Ram.hpp"
#include "Cache.hpp"
#include "Cpu.hpp"

bool useL2 = true;

void runDaxpy(Cpu& cpu, int n, double D, int sz) {
    std::vector<unsigned int> a(n), b(n), c(n);
    for (int i = 0; i < n; ++i) {
        a[i] = i * sz;
        b[i] = (i + n) * sz;
        c[i] = (i + 2 * n) * sz;
        cpu.storeDouble(a[i], i);
        cpu.storeDouble(b[i], 2 * i);
        cpu.storeDouble(c[i], 0);
    }

    for (int i = 0; i < n; ++i) {
        double val = cpu.addDouble(cpu.multDouble(D, cpu.loadDouble(a[i])), cpu.loadDouble(b[i]));
        cpu.storeDouble(c[i], val);
    }
}

void runMatrixMultiply(Cpu& cpu, int dim, int sz) {
    int bytesPerMat = dim * dim * sz;
    unsigned int A = 0;
    unsigned int B = A + bytesPerMat;
    unsigned int C = B + bytesPerMat;

    for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j) {
            double aVal = i * dim + j;
            cpu.storeDouble(A + (i * dim + j) * sz, aVal);
            cpu.storeDouble(B + (i * dim + j) * sz, 2 * aVal);
            cpu.storeDouble(C + (i * dim + j) * sz, 0);
        }

    for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j) {
            double sum = 0;
            for (int k = 0; k < dim; ++k) {
                double a = cpu.loadDouble(A + (i * dim + k) * sz);
                double b = cpu.loadDouble(B + (k * dim + j) * sz);
                sum = cpu.addDouble(sum, cpu.multDouble(a, b));
            }
            cpu.storeDouble(C + (i * dim + j) * sz, sum);
        }
}

void runMatrixMultiplyBlocked(Cpu& cpu, int dim, int sz, int blockSize) {
    int bytesPerMat = dim * dim * sz;
    unsigned int A = 0;
    unsigned int B = A + bytesPerMat;
    unsigned int C = B + bytesPerMat;

    for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j) {
            double aVal = i * dim + j;
            cpu.storeDouble(A + (i * dim + j) * sz, aVal);
            cpu.storeDouble(B + (i * dim + j) * sz, 2 * aVal);
            cpu.storeDouble(C + (i * dim + j) * sz, 0);
        }

    for (int ii = 0; ii < dim; ii += blockSize)
        for (int jj = 0; jj < dim; jj += blockSize)
            for (int kk = 0; kk < dim; kk += blockSize)
                for (int i = ii; i < std::min(ii + blockSize, dim); ++i)
                    for (int j = jj; j < std::min(jj + blockSize, dim); ++j) {
                        double sum = cpu.loadDouble(C + (i * dim + j) * sz);
                        for (int k = kk; k < std::min(kk + blockSize, dim); ++k) {
                            double a = cpu.loadDouble(A + (i * dim + k) * sz);
                            double b = cpu.loadDouble(B + (k * dim + j) * sz);
                            sum = cpu.addDouble(sum, cpu.multDouble(a, b));
                        }
                        cpu.storeDouble(C + (i * dim + j) * sz, sum);
                    }
}

int main(int argc, char** argv) {
    int cacheSize = 65536;
    int blockSize = 64;
    int nWay = 2;
    std::string policy = "LRU";
    std::string algorithm = "mxm_block";
    int dimension = 480;
    bool printResult = false;
    int blockingFactor = 32;

    int opt;
    while ((opt = getopt(argc, argv, "c:b:n:r:a:d:pf:")) != -1) {
        switch (opt) {
            case 'c': cacheSize = std::stoi(optarg); break;
            case 'b': blockSize = std::stoi(optarg); break;
            case 'n': nWay = std::stoi(optarg); break;
            case 'r': policy = optarg; break;
            case 'a': algorithm = optarg; break;
            case 'd': dimension = std::stoi(optarg); break;
            case 'p': printResult = true; break;
            case 'f': blockingFactor = std::stoi(optarg); break;
            default: std::cerr << "Invalid arguments\n"; exit(1);
        }
    }

    int bytesNeeded = 0;
    if (algorithm == "daxpy") bytesNeeded = dimension * 3 * sizeof(double);
    else bytesNeeded = dimension * dimension * 3 * sizeof(double);
    bytesNeeded += blockSize;

    int numSets = (cacheSize / blockSize) / nWay;
    Ram ram(bytesNeeded, blockSize);

    Cache cache(cacheSize, blockSize, nWay, policy, ram, useL2);
    Cpu cpu(cache, blockSize, numSets, useL2);

    printf("INPUTS====================================\n");
    printf("Ram Size = %d bytes\n", ram.getSize());
    printf("Cache Size = %d bytes\n", cacheSize);
    printf("Block Size = %d bytes\n", blockSize);
    printf("Total Blocks in Cache = %d\n", cacheSize / blockSize);
    printf("Associativity = %d\n", nWay);
    printf("Number of Sets = %d\n", numSets);
    printf("Replacement Policy = %s\n", policy.c_str());
    printf("Algorithm = %s\n", algorithm.c_str());
    printf("MXM Blocking Factor = %d\n", blockingFactor);
    printf("Matrix or Vector dimension = %d\n", dimension);

    if (algorithm == "daxpy") {
        runDaxpy(cpu, dimension, 3.0, sizeof(double));
        if (printResult) {
            printf("DAXPY Output C = [");
            for (int i = 0; i < dimension; ++i) {
                unsigned int addr = (i + 2 * dimension) * sizeof(double);
                printf("%.0f", cpu.loadDouble(addr));
                if (i != dimension - 1) printf(", ");
            }
            printf("]\n");
        }
    }
    else if (algorithm == "mxm") {
        runMatrixMultiply(cpu, dimension, sizeof(double));
        if (printResult) {
            int bytesPerMat = dimension * dimension * sizeof(double);
            unsigned int C = 2 * bytesPerMat;
            printf("Matrix C (%dx%d):\n", dimension, dimension);
            for (int i = 0; i < dimension; ++i) {
                for (int j = 0; j < dimension; ++j) {
                    double val = cpu.loadDouble(C + (i * dimension + j) * sizeof(double));
                    printf("%6.0f ", val);
                }
                printf("\n");
            }
        }
    }
    else if (algorithm == "mxm_block") {
        runMatrixMultiplyBlocked(cpu, dimension, sizeof(double), blockingFactor);
        if (printResult) {
            int bytesPerMat = dimension * dimension * sizeof(double);
            unsigned int C = 2 * bytesPerMat;
            printf("Blocked Matrix C (%dx%d):\n", dimension, dimension);
            for (int i = 0; i < dimension; ++i) {
                for (int j = 0; j < dimension; ++j) {
                    double val = cpu.loadDouble(C + (i * dimension + j) * sizeof(double));
                    printf("%6.0f ", val);
                }
                printf("\n");
            }
        }
    }
    else {
        std::cerr << "Unknown algorithm\n";
        return 1;
    }

    printf("RESULTS====================================\n");
    cpu.printStats();

    return 0;
}