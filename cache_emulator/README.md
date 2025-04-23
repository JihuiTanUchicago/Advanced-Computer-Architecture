# Cache Emulator Analysis

### DAXPY (n = 9, D = 3)

**A =** [0, 1, 2, 3, 4, 5, 6, 7, 8]  
**B =** [0, 2, 4, 6, 8, 10, 12, 14, 16]  
**C = D * A + B =** [0, 5, 10, 15, 20, 25, 30, 35, 40]

```
$ ./cache-sim -a daxpy -d 9 -n 1 -c 65536 -b 64 -r LRU -p
INPUTS====================================
Ram Size = 256 bytes
Cache Size = 65536 bytes
Block Size = 64 bytes
Total Blocks in Cache = 1024
Associativity = 1
Number of Sets = 1024
Replacement Policy = LRU
Algorithm = daxpy
MXM Blocking Factor = 32
Matrix or Vector dimension = 9
DAXPY Output C = [0, 5, 10, 15, 20, 25, 30, 35, 40]
RESULTS====================================
Instruction count: 81
Read hits: 27
Read misses: 0
Read miss rate: 0.00%
Write hits: 32
Write misses: 4
Write miss rate: 11.11%
```

### Matrix-Matrix Multiplication (9x9)

**A[i][j] = i + j**  
**B[i][j] = 2(i + j)**  
**C = A × B, row-major layout**  

```
$ ./cache-sim -a mxm -d 3 -p
INPUTS====================================
Ram Size = 256 bytes
Cache Size = 65536 bytes
Block Size = 64 bytes
Total Blocks in Cache = 1024
Associativity = 2
Number of Sets = 512
Replacement Policy = LRU
Algorithm = mxm
MXM Blocking Factor = 32
Matrix or Vector dimension = 3
Matrix C (3x3):
    30     36     42 
    84    108    132 
   138    180    222 
RESULTS====================================
Instruction count: 153
Read hits: 63
Read misses: 0
Read miss rate: 0.00%
Write hits: 32
Write misses: 4
Write miss rate: 11.11%
```

### Blocked Matrix-Matrix Multiplication (block = 3)

```
$ ./cache-sim -a mxm_block -d 3 -p
INPUTS====================================
Ram Size = 256 bytes
Cache Size = 65536 bytes
Block Size = 64 bytes
Total Blocks in Cache = 1024
Associativity = 2
Number of Sets = 512
Replacement Policy = LRU
Algorithm = mxm_block
MXM Blocking Factor = 32
Matrix or Vector dimension = 3
Blocked Matrix C (3x3):
    30     36     42 
    84    108    132 
   138    180    222 
RESULTS====================================
Instruction count: 162
Read hits: 72
Read misses: 0
Read miss rate: 0.00%
Write hits: 32
Write misses: 4
Write miss rate: 11.11%
```

## Cache Associativity Analysis

| Associativity | Instructions | Read Hits   | Read Misses | Read Miss % | Write Hits | Write Misses | Write Miss % |
|---------------|--------------|-------------|--------------|--------------|-------------|----------------|----------------|
| 1             | 449,971,200  | 222,216,486 | 2,423,514    | 1.08%        | 3,630,720   | 516,480        | 12.45%         |
| 2             | 449,971,200  | 223,698,840 |   941,160    | 0.42%        | 4,060,800   |  86,400        | 2.08%          |
| 4             | 449,971,200  | 223,747,200 |   892,800    | 0.40%        | 4,060,800   |  86,400        | 2.08%          |
| 8             | 449,971,200  | 223,747,200 |   892,800    | 0.40%        | 4,060,800   |  86,400        | 2.08%          |
| 16            | 449,971,200  | 223,747,200 |   892,800    | 0.40%        | 4,060,800   |  86,400        | 2.08%          |
| 1024 (FA)     | 449,971,200  | 223,747,200 |   892,800    | 0.40%        | 4,060,800   |  86,400        | 2.08%          |

8-way associativity offers a strong balance between hardware complexity and cache performance. 
Going beyond 8-way yields no gain and will likely saturate performance.

## Memory Block Size Analysis

| Block Size | Instructions | Read Hits   | Read Misses | Read Miss % | Write Hits | Write Misses | Write Miss % |
|------------|--------------|-------------|--------------|--------------|-------------|----------------|----------------|
| 8          | 449,971,200  | 217,110,720 | 7,529,280    | 3.35%        | 3,456,000   | 691,200        | 16.67%         |
| 16         | 449,971,200  | 220,875,360 | 3,764,640    | 1.68%        | 3,801,600   | 345,600        | 8.33%          |
| 32         | 449,971,200  | 222,757,680 | 1,882,320    | 0.84%        | 3,974,400   | 172,800        | 4.17%          |
| 64         | 449,971,200  | 223,698,840 |   941,160    | 0.42%        | 4,060,800   |  86,400        | 2.08%          |
| 128        | 449,971,200  | 224,169,420 |   470,580    | 0.21%        | 4,104,000   |  43,200        | 1.04%          |
| 256        | 449,971,200  | 224,404,710 |   235,290    | 0.10%        | 4,125,600   |  21,600        | 0.52%          |
| 512        | 449,971,200  | 222,728,738 | 1,911,262    | 0.85%        | 3,734,992   | 412,208        | 9.94%          |
| 1024       | 449,971,200  | 219,352,185 | 5,287,815    | 2.35%        | 2,932,456   | 1,214,744      | 29.29%         |

```
As the block size increases from 8 to 256 bytes, both read and write miss rates steadily decrease. This is due to better exploitation of spatial locality—each cache block fetch brings in more adjacent data that is likely to be accessed soon, especially in the innermost loops of blocked matrix multiplication where memory access is linear and stride-1 along certain dimensions. In these cases, larger blocks mean fewer compulsory and capacity misses since more useful data gets preloaded.

The optimal point appears around 128–256 bytes, where miss rates are minimized (read miss ≈ 0.10%, write miss ≈ 0.52%). Beyond this point, as block size grows to 512 and then 1024, miss rates begin to rise again. This happens because fewer total blocks can now fit in the same cache size. At 1024 bytes per block, only 64 blocks exist in the entire cache, increasing conflict misses and cache pollution—especially for write traffic, which becomes significantly worse (write miss rate jumps to 29.29%).
```

## Total Cache Size Analysis

| Cache Size | Instructions | Read Hits   | Read Misses | Read Miss % | Write Hits | Write Misses | Write Miss % |
|------------|--------------|-------------|--------------|--------------|-------------|----------------|----------------|
| 4096       | 449,971,200  | 109,688,400 | 114,951,600  | 51.17%       | 0           | 4,147,200      | 100.00%        |
| 8192       | 449,971,200  | 206,585,790 | 18,054,210   | 8.04%        | 0           | 4,147,200      | 100.00%        |
| 16384      | 449,971,200  | 222,907,245 | 1,732,755    | 0.77%        | 3,225,600   | 921,600        | 22.22%         |
| 32768      | 449,971,200  | 223,321,204 | 1,318,796    | 0.59%        | 3,946,112   | 201,088        | 4.85%          |
| 65536      | 449,971,200  | 223,698,840 |   941,160    | 0.42%        | 4,060,800   | 86,400         | 2.08%          |
| 131072     | 449,971,200  | 223,736,248 |   903,752    | 0.40%        | 4,060,800   | 86,400         | 2.08%          |
| 262144     | 449,971,200  | 224,019,377 |   620,623    | 0.28%        | 4,060,800   | 86,400         | 2.08%          |
| 524288     | 449,971,200  | 224,133,224 |   506,776    | 0.23%        | 4,060,800   | 86,400         | 2.08%          |

## Problem Size and Cache Thrashing

### Associativity = 2

| Matrix Dimension | MxM Method  | Blocking Factor | Instructions | Read Hits   | Read Misses | Read Miss % | Write Hits  | Write Misses | Write Miss % |
|------------------|-------------|-----------------|--------------|-------------|-------------|-------------|-------------|--------------|--------------|
| 480              | Regular     | -               | 443,289,600  | 107,035,281 | 114,148,719 | 51.61%      | 604,800     | 316,800      | 34.38%       |
| 480              | Blocked     | 8               | 470,707,200  | 232,690,141 | 2,317,859   | 0.99%       | 14,428,800  | 86,400       | 0.59%        |
| 488              | Regular     | -               | 465,809,664  | 217,464,849 | 14,963,695  | 6.44%       | 833,504     | 119,072      | 12.50%       |
| 488              | Blocked     | 8               | 494,625,088  | 244,625,824 | 2,329,504   | 0.94%       | 15,151,912  | 89,304       | 0.59%        |
| 512              | Regular     | -               | 537,919,488  | 133,891,584 | 134,543,872 | 50.12%      | 0           | 1,048,576    | 100.00%      |
| 512              | Blocked     | 8               | 571,211,776  | 281,121,792 | 4,090,880   | 1.43%       | 16,515,072  | 1,048,576    | 5.97%        |



```
1. These results suggest that even small changes in problem size (such as 480×480 to 488×488) can significantly affect cache performance due to how the cache sets are indexed.
```

```
2. When using the blocked matrix multiply algorithm, we observe a drastic improvement in the cache miss rate for the 512×512 problem size.
The blocked matrix multiplication algorithm divides the matrix into smaller blocks that fit more effectively within the cache. This method improves spatial locality and reduces cache thrashing because each block can be reused multiple times before being evicted from the cache. For larger matrices (512×512), blocking allows us to maximize cache utilization, leading to significantly lower cache misses.
```

### Associativity = 8

| Matrix Dimension | MxM Type | Block | Instructions | Read Hits   | Read Misses | Read Miss % | Write Hits | Write Misses | Write Miss % |
|------------------|----------|-------|--------------|-------------|-------------|-------------|------------|--------------|--------------|
| 480              | Regular  | -     | 443,289,600  | 107,118,315 | 114,065,685 | 51.57%      | 604,800    | 316,800      | 34.38%       |
| 480              | Blocked  | 8     | 470,707,200  | 232,615,590 | 2,392,410   | 1.02%       | 14,428,800 | 86,400       | 0.60%        |
| 488              | Regular  | -     | 465,809,664  | 217,871,996 | 14,556,548  | 6.26%       | 833,504    | 119,072      | 12.50%       |
| 488              | Blocked  | 8     | 494,625,088  | 244,751,209 | 2,204,119   | 0.89%       | 15,151,912 | 89,304       | 0.59%        |
| 512              | Regular  | -     | 537,919,488  | 133,892,096 | 134,543,360 | 50.12%      | 688,128    | 360,448      | 34.38%       |
| 512              | Blocked  | 8     | 571,211,776  | 281,429,120 | 3,783,552   | 1.33%       | 17,465,344 | 98,304       | 0.56%        |


### Fully Associative

| Matrix Dimension | MxM Method | Blocking Factor | Instructions | Read Hits   | Read Misses | Read Miss % | Write Hits   | Write Misses | Write Miss % |
|------------------|------------|-----------------|--------------|-------------|-------------|-------------|--------------|--------------|--------------|
| 480              | Regular    | -               | 443,289,600  | 207,331,202 | 13,852,798  | 6.26%       | 806,400      | 115,200      | 12.50%       |
| 480              | Blocked    | 8               | 470,707,200  | 233,222,400 | 1,785,600   | 0.76%       | 14,428,800   | 86,400       | 0.60%        |
| 488              | Regular    | -               | 465,809,664  | 217,871,994 | 14,556,550  | 6.26%       | 833,504      | 119,072      | 12.50%       |
| 488              | Blocked    | 8               | 494,625,088  | 245,079,944 | 1,875,384   | 0.76%       | 15,151,912   | 89,304       | 0.59%        |
| 512              | Regular    | -               | 537,919,488  | 251,625,474 | 16,809,982  | 6.26%       | 917,504      | 131,072      | 12.50%       |
| 512              | Blocked    | 8               | 571,211,776  | 280,985,600 | 4,227,072   | 1.48%       | 17,465,344   | 98,304       | 0.56%        |


```
3. When comparing Table 5 (associativity = 8) and Table 6 (fully associative), we see an improvement in performance as associativity increases.
Higher associativity reduces conflict misses, as more cache sets are available for each memory block, which reduces the likelihood of cache lines evicting one another. Fully associative caches eliminate all conflict misses, but at the cost of slower cache lookups and higher power consumption. Despite theoretical benefit, fully associative caches might not be elected due to latency.
```

```
4. I would suggest 1) using blocked matrix multiplication technique and 2) making sure matrix data is well aligned in memory.
```

## Replacement Policy Comparison

| Replacement Policy | Instructions  | Read Hits   | Read Misses  | Read Miss % | Write Hits  | Write Misses | Write Miss % |
|--------------------|---------------|-------------|--------------|-------------|-------------|--------------|--------------|
| Random             | 443,289,600   | 123,157,058 | 98,026,942   | 44.32%      | 622,599     | 299,001      | 32.50%       |
| FIFO               | 443,289,600   | 107,131,631 | 114,052,369  | 51.56%      | 606,848     | 314,752      | 34.38%       |
| LRU                | 443,289,600   | 107,035,281 | 114,148,719  | 51.61%      | 604,800     | 316,800      | 34.38%       |


## L1 + L2 

### Table 8  –  Miss Rates per Cache Level  
Blocked MXM (d = 480, B = 32)

| Config | L1 Read Miss % | L1 Write Miss % | L2 Read Miss % | L2 Write Miss % |
|--------|----------------|-----------------|----------------|-----------------|
| **L1 Only** | 51.67 % | 34.38 % | — | — |
| **L1 + L2** | 51.74 % | 34.38 % | 24.75 % | 34.38 % |

---

### Memory‑access Totals & Hits  
Blocked MXM (d = 480, B = 32)

| Config | RAM Accesses<sup>†</sup> | Read Hits (L1 + L2) | Write Hits (L1 + L2) |
|--------|-------------------------|---------------------|----------------------|
| **L1 Only** | **114 593 700** | 106 907 100 | 604 800 |
| **L1 + L2** | **28 642 422**  | 192 858 378 | 604 800 |
