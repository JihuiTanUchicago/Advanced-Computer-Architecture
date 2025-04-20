### Cascade Lake Background Information
- Default Page Size: 4096 = 4 KB
- L1 DTLB: 64 entries for 4 KiB page translations; 4-way set associative
- L2 STLB: 1536 entries for 4 KiB page translations; 12-way set associative


### Compilation and Run
Compile with:
```
g++ -std=c++17 diagnosing_tlb_induced_slowdowns.cpp -lpapi -o tlb_stress
```
Then run:
```
PAPI_EVENTS=PAPI_TLB_DM,PAPI_L1_DCM ./tlb_stress
```

### Runtime Measurement
```
Pages: 64, Stride: 16, Time: 28404 ns, sum: 4096
Pages: 4096, Stride: 1024, Time: 61694 ns, sum: 4096
```

### TLB Statistics
```
{
  "cpu in mhz":"3000",
  "threads":[
    {
      "id":"139681565112192",
      "regions":[
        {
          "tlb_stress Pages=1;Stride=1":{
            "region_count":"1",
            "cycles":"102782",
            "PAPI_TLB_DM":"18",
            "PAPI_L1_DCM":"4445"
          }
        },
        {
          "tlb_stress Pages=4096;Stride=1024":{
            "region_count":"1",
            "cycles":"192810",
            "PAPI_TLB_DM":"229",
            "PAPI_L1_DCM":"4191"
          }
        }
      ]
    }
  ]
}
```

### Memory Access Pattern and How It Stresses the TLB
- L1 DTLB has maximum 64 entries for 4 KiB page translations. Thus, we can create an integer array that
spans multiple pages(much greater than 64 pages) to force TLB misses.
- We setup 2 trials, each trail calculating array sum to `4096`, but `the 1st trial (Pages=64, Stride=16)`
has less TLB misses due to accessing multiple integers within the same page, whereas `the 2nd tiral (Pages=4096, Stride=1024)`
accesses 1 integer per page.
- We make sure to minimalize the effect of L1 Data Cache by setting `strides >= 16` so that each array access is a guaranteed L1 data cache miss.

### Analysis
- We observed almost equivalent L1 data cache misses (1st Trial: 4445, 2nd Trial: 4191) with the 1st trial even having more L1 data cache misses.
- We also observed 2nd trial was significantly slower (61694 ns) compared to 1st trial (28404 ns). The TLB misses also correspondingly increased in 2nd Trial (229) compared to 1st Trial (18).
- The performance gap is significant, and the data mentioned above indicate that 2nd trial experienced performance degradation due to more TLB misses.

### Could using huge pages (or smaller working sets) reduce the slowdown?
- Having huge pages could reduce the slowdown. Imagine a page being 16 MB, then it could fit data in 4096 pages of 4 KiB pages into 1 big page, and therefore this huge page only takes up 1 entry in TLB. Then we would expect both arrays in 1st and 2nd trials to fit into a 16 MB, then there would be no TLB miss for the calculations.
- Equivalently, smaller working sets also reduce the slowdown. 1 KiB page could fit 1024 4-byte integers. This is already evident comparing Trial 2 and Trial 1.