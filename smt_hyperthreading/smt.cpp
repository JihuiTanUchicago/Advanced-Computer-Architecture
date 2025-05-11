#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <sched.h>
#include <unistd.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/mach_traps.h>
#include <pthread.h>


using namespace std;
using namespace chrono;

// Helper to set CPU affinity on macOS (works on ARM and Intel with thread affinity tags)
void pin_thread_to_core(int core_id) {
#ifdef __APPLE__
    thread_port_t threadport = pthread_mach_thread_np(pthread_self());
    thread_affinity_policy_data_t policy = {core_id};
    thread_policy_set(threadport, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
#endif
}

// Compute-bound workload
void compute_bound(size_t iterations) {
    pin_thread_to_core(0);
    volatile double result = 0.0;
    for (size_t i = 0; i < iterations; ++i) {
        result += sin(i) * cos(i);
    }
}

// Memory-latency-bound workload (pointer chasing)
void memory_latency_bound(vector<size_t>& next_indices, size_t start, size_t steps) {
    pin_thread_to_core(0);
    size_t idx = start;
    for (size_t i = 0; i < steps; ++i) {
        idx = next_indices[idx];
    }
}

void run_experiment() {
    size_t compute_iterations = 1e9;
    size_t memory_steps = 1e8;
    size_t memory_size = 1e7;
    
    // Prepare pointer-chasing structure
    vector<size_t> next_indices(memory_size);
    for (size_t i = 0; i < memory_size - 1; ++i) next_indices[i] = i + 1;
    next_indices[memory_size - 1] = 0;

    // Single-threaded Compute-bound
    cout << "Running Compute-bound with 1 thread..." << endl;
    auto start = steady_clock::now();
    compute_bound(compute_iterations);
    auto end = steady_clock::now();
    cout << "Time: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    // Two-threaded Compute-bound
    cout << "Running Compute-bound with 2 threads..." << endl;
    start = steady_clock::now();
    thread t1(compute_bound, compute_iterations / 2);
    thread t2(compute_bound, compute_iterations / 2);
    t1.join();
    t2.join();
    end = steady_clock::now();
    cout << "Time: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    // Single-threaded Memory-latency-bound
    cout << "Running Memory-latency-bound with 1 thread..." << endl;
    start = steady_clock::now();
    memory_latency_bound(next_indices, 0, memory_steps);
    end = steady_clock::now();
    cout << "Time: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    // Two-threaded Memory-latency-bound
    cout << "Running Memory-latency-bound with 2 threads..." << endl;
    start = steady_clock::now();
    thread t3(memory_latency_bound, ref(next_indices), 0, memory_steps / 2);
    thread t4(memory_latency_bound, ref(next_indices), 0, memory_steps / 2);
    t3.join();
    t4.join();
    end = steady_clock::now();
    cout << "Time: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
}

int main() {
    run_experiment();
    return 0;
}
