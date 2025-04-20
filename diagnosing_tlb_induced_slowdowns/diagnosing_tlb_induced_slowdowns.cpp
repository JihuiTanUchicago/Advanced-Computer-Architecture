#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <string>

extern "C" {
#include <papi.h>
}

using namespace std;
using Clock = chrono::high_resolution_clock;

const int PAGE_SIZE_BYTES = 4096;
const int INT_PER_PAGE = PAGE_SIZE_BYTES / sizeof(int);

void runExperiment(size_t totalPages, size_t stride, const char* title) {
    size_t arraySize = totalPages * INT_PER_PAGE;
    vector<int> array(arraySize, 1);
    volatile int sum = 0;

    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret < 0) {
        cerr << "PAPI_library_init failed: " << PAPI_strerror(ret) << endl;
        exit(1);
    }

    ret = PAPI_hl_region_begin(title);
    if (ret != PAPI_OK) {
        cerr << "PAPI_hl_region_begin failed: " << PAPI_strerror(ret) << endl;
        exit(1);
    }

    auto start = Clock::now();
    for (size_t i = 0; i < arraySize; i += stride) {
        sum += array[i];
    }
    auto end = Clock::now();

    ret = PAPI_hl_region_end(title);
    if (ret != PAPI_OK) {
        cerr << "PAPI_hl_region_end failed: " << PAPI_strerror(ret) << endl;
        exit(1);
    }

    auto duration_ns = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

    cout << "Pages: " << totalPages
        << ", Stride: " << stride
        << ", Time: " << duration_ns << " ns"
        << ", sum: " << sum
        << endl;
}

int main() {
    cout << "TLB Stress Test with PAPI (High-Level API)" << endl;
    string title1 = "tlb_stress Pages=" + to_string(1) + ";Stride=" + to_string(1);
    runExperiment(64, 16, title1.c_str());
    string title2 = "tlb_stress Pages=" + to_string(4096) + ";Stride=" + to_string(1024);
    runExperiment(4096, 1024, title2.c_str());
    return 0;
}
