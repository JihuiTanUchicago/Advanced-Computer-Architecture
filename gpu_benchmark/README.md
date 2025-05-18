## How to Run

First:

```bash
sinteractive --partition=caslake --nodes 1 --ntasks-per-node 1 --cpus-per-task 8 --ntasks 1 --time=01:00:00 --account=mpcs52018
```

Then
```
module load cuda
```


```bash
bash benchmark.sh
```

## Benchmark Results

| Implementation         | Runtime (seconds) | Speedup vs Serial |
|-----------------------|-------------------|-------------------|
| Serial CPU             | 0.012992          | 1.00×             |
| Parallel OMP CPU       | 0.001912          | 6.80×             |
| GPU Single Thread      | 0.000026          | 499.69×           |
| GPU Coarse-Grained     | 0.000023          | 565.74×           |
| GPU Fine-Grained       | 0.000027          | 481.18×           |