#ifndef CPU_HPP
#define CPU_HPP

#include "Cache.hpp"
#include "Address.hpp"

class Cpu {
private:
    Cache& cache;  // This can refer to the L1 cache, or to a cache that includes L2 if `useL2Cache` is true
    int blockSize;
    int numSets;
    bool useL2Cache;  // Flag to indicate whether L2 cache is used

    // Instruction counters
    int numAdd = 0;
    int numMult = 0;
    int numLoad = 0;
    int numStore = 0;

public:
    // Constructor: Pass in a flag to enable L2 cache
    Cpu(Cache& cache, int blockSize, int numSets, bool useL2Cache = false)
        : cache(cache), blockSize(blockSize), numSets(numSets), useL2Cache(useL2Cache) {}

    double loadDouble(unsigned int addr) {
        numLoad++;
        Address address(addr, blockSize, numSets);
        return cache.getDouble(address);
    }

    void storeDouble(unsigned int addr, double value) {
        numStore++;
        Address address(addr, blockSize, numSets);
        cache.setDouble(address, value);
    }

    double addDouble(double a, double b) {
        numAdd++;
        return a + b;
    }

    double multDouble(double a, double b) {
        numMult++;
        return a * b;
    }

    void printStats() const {
        // Access L1 cache statistics
        int l1Reads = cache.l1ReadHits + cache.l1ReadMisses;
        int l1Writes = cache.l1WriteHits + cache.l1WriteMisses;

        printf("L1 Cache Statistics:\n");
        printf("Read hits: %d\n", cache.l1ReadHits);
        printf("Read misses: %d\n", cache.l1ReadMisses);
        printf("Read miss rate: %.2f%%\n", l1Reads == 0 ? 0.0 : 100.0 * cache.l1ReadMisses / l1Reads);
        printf("Write hits: %d\n", cache.l1WriteHits);
        printf("Write misses: %d\n", cache.l1WriteMisses);
        printf("Write miss rate: %.2f%%\n", l1Writes == 0 ? 0.0 : 100.0 * cache.l1WriteMisses / l1Writes);

        // If L2 is enabled, print L2 cache statistics
        if (useL2Cache) {
            int l2Reads = cache.l2ReadHits + cache.l2ReadMisses;
            int l2Writes = cache.l2WriteHits + cache.l2WriteMisses;

            printf("\nL2 Cache Statistics:\n");
            printf("Read hits: %d\n", cache.l2ReadHits);
            printf("Read misses: %d\n", cache.l2ReadMisses);
            printf("Read miss rate: %.2f%%\n", l2Reads == 0 ? 0.0 : 100.0 * cache.l2ReadMisses / l2Reads);
            printf("Write hits: %d\n", cache.l2WriteHits);
            printf("Write misses: %d\n", cache.l2WriteMisses);
            printf("Write miss rate: %.2f%%\n", l2Writes == 0 ? 0.0 : 100.0 * cache.l2WriteMisses / l2Writes);
        }
    }
};

#endif