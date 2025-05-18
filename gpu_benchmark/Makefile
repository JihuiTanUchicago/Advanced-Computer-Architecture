all: histogram_serial histogram_omp histogram_gpu_single histogram_gpu_coarse histogram_gpu_fine

histogram_serial: histogram_serial.c
	gcc -fopenmp -o histogram_serial histogram_serial.c -O2

histogram_omp: histogram_omp.c
	gcc -fopenmp -o histogram_omp histogram_omp.c -O2

histogram_gpu_single: histogram_gpu_single.cu
	nvcc -o histogram_gpu_single histogram_gpu_single.cu

histogram_gpu_coarse: histogram_gpu_coarse.cu
	nvcc -o histogram_gpu_coarse histogram_gpu_coarse.cu

histogram_gpu_fine: histogram_gpu_fine.cu
	nvcc -o histogram_gpu_fine histogram_gpu_fine.cu

clean:
	rm -f histogram_serial histogram_omp histogram_gpu_single histogram_gpu_coarse histogram_gpu_fine