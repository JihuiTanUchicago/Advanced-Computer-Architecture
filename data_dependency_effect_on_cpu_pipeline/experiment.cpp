#include <papi.h>
#include <cassert>
#include <cstdio>

constexpr int kArraySize   = 1 << 20;   // 1 M doubles ≈ 8 MiB
constexpr int kNumRuns     = 3;
constexpr int kMaxCounters = 3;

double a[kArraySize]{}, b[kArraySize]{}, c[kArraySize]{};

enum CounterIdx { totCyc, totIns, stall, numCnt };

long long values[numCnt]{};

void startCounter(int& eventSet)
{
    eventSet = PAPI_NULL;
    assert(PAPI_create_eventset(&eventSet) == PAPI_OK);

    int wanted[kMaxCounters] = { PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_RES_STL };
    int added                = 0;

    for (int ev : wanted)
        if (PAPI_query_event(ev) == PAPI_OK)
            PAPI_add_event(eventSet, ev), ++added;

    assert(added >= 2);                
    PAPI_start(eventSet);
}

void stopCounter(int eventSet, long long* accum)
{
    long long sample[kMaxCounters]{};
    PAPI_stop(eventSet, sample);
    PAPI_cleanup_eventset(eventSet);

    for (int i = 0; i < kMaxCounters; ++i) accum[i] += sample[i];
}

inline void loopIndependent()
{
    for (int i = 0; i < kArraySize; ++i) b[i] = c[i] + 1.0;
}
inline void loopSingleDep()
{
    for (int i = 1; i < kArraySize; ++i) a[i] = a[i - 1] + 1.0;
}
inline void loopDoubleDep()
{
    for (int i = 2; i < kArraySize; ++i) a[i] = a[i - 1] + a[i - 2] + 1.0;
}

template <typename Kernel>
void benchmark(const char* tag, Kernel kernel)
{
    long long accum[kMaxCounters]{};

    for (int run = 0; run < kNumRuns; ++run)
    {
        int eventSet;
        startCounter(eventSet);
        kernel();
        stopCounter(eventSet, accum);
    }

    double cycles = accum[totCyc] / static_cast<double>(kNumRuns);
    double insts  = accum[totIns] / static_cast<double>(kNumRuns);
    double stalls = accum[stall]   ? accum[stall] / static_cast<double>(kNumRuns) : 0.0;

    printf("%-13s %10.0f %10.0f %10.0f %6.2f %6.1f%%\n",
           tag, cycles, insts, stalls, cycles / insts,
           stalls ? 100.0 * stalls / cycles : 0.0);
}

int main()
{
    assert(PAPI_library_init(PAPI_VER_CURRENT) == PAPI_VER_CURRENT);

    puts("loop            cycles        insts       stalls    CPI   %stall");
    benchmark("independent", loopIndependent);
    benchmark("1-dep",       loopSingleDep);
    benchmark("2-dep",       loopDoubleDep);
}
