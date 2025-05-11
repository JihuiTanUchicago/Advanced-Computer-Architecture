#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <sched.h>
#include <pthread.h>
#include <algorithm>
#include <random>
#include <cmath>

using namespace std;
using namespace chrono;

void pin_thread_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

//compute-bound workload: heavy arithmetic
void compute_bound(size_t iterations) {
    pin_thread_to_core(0);
    volatile double result = 0.0;
    for (size_t i = 0; i < iterations; ++i) {
        result += sin(i) * cos(i);
    }
}

volatile size_t sink = 0;

//memory-latency-bound workload: random pointer chasing
void memory_latency_bound(vector<size_t>& indices, size_t start, size_t steps) {
    pin_thread_to_core(0);
    size_t idx = start;
    for (size_t i = 0; i < steps; ++i) {
        idx = indices[idx];
    }

    sink = idx;
}

void run_experiment() {
    size_t compute_iterations = 1e9;
    size_t memory_steps = 1e10;
    size_t memory_size = 1e7;

    vector<size_t> indices(memory_size);
    for (size_t i = 0; i < memory_size; ++i) indices[i] = i;
    shuffle(indices.begin(), indices.end(), default_random_engine(42));  //ensure random access
    for (size_t i = 0; i < memory_size - 1; ++i) indices[indices[i]] = indices[(i + 1) % memory_size];
    indices[indices[memory_size - 1]] = indices[0];  //close loop

    auto start = steady_clock::now();
    compute_bound(compute_iterations);
    auto end = steady_clock::now();
    cout << "Compute-bound 1 thread: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    start = steady_clock::now();
    thread t1(compute_bound, compute_iterations / 2);
    thread t2(compute_bound, compute_iterations / 2);
    t1.join();
    t2.join();
    end = steady_clock::now();
    cout << "Compute-bound 2 threads: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    start = steady_clock::now();
    memory_latency_bound(indices, indices[0], memory_steps);
    end = steady_clock::now();
    cout << "Memory-bound 1 thread: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    start = steady_clock::now();
    thread t3(memory_latency_bound, ref(indices), indices[0], memory_steps / 2);
    thread t4(memory_latency_bound, ref(indices), indices[0], memory_steps / 2);
    t3.join();
    t4.join();
    end = steady_clock::now();
    cout << "Memory-bound 2 threads: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
}

int main() {
    run_experiment();
}
