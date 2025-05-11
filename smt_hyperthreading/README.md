### Compilation and Run
```
g++ -O2 -pthread -o smt smt.cpp
./smt
```

### Result
```
Compute-bound 1 thread: 63742 ms
Compute-bound 2 threads: 57283 ms
Memory-bound 1 thread: 40669 ms
Memory-bound 2 threads: 40614 ms
```

### Analysis
SMT provides noticeable benefits for compute-bound workloads (10% improvement) but no measurable improvement for memory-latency-bound workloads. SMT allows better utilization of idle execution pipeline slots when one thread stalls on instruction scheduling or dependency resolution. In contrast, memory-latency-bound workloads have to contend for the same memory bandwidth and cache resources, leading to stalls dominated by memory latency that SMT cannot hide. This shows that SMT is most effective when the bottleneck is CPU execution resources because each core has their own compute units, but SMT is ineffective when memory system is shared.
