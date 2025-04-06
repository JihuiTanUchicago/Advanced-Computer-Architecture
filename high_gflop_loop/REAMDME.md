### Approach
Increment `acc` by 0.005 each time for a loop that runs N times, where N is a very large number.
Compare the actual result and the expected result. See output for more details.

### Compilation
Run in macbook air that has Apple M3 Chip.
```
gcc -O3 -march=native -ffast-math -funroll-loops high_gflop_loop.c -o benchmark -lm
```

### Result
```
Time: 0.015198 s
Performance: 65.798131 GFLOP/s
Final acc: 499999.999893 (to prevent optimization)
```