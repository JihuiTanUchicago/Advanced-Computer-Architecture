#!/bin/bash
module load cuda

make clean && make

./histogram_serial       | tee -a benchmark_results.md
./histogram_omp          | tee -a benchmark_results.md
./histogram_gpu_single   | tee -a benchmark_results.md
./histogram_gpu_coarse   | tee -a benchmark_results.md
./histogram_gpu_fine     | tee -a benchmark_results.md