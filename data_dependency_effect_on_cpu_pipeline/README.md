### Compilation
Compile with:
```
g++ experiment.cpp -lpapi -o experiment
```

Run:
```
./experiment
```

### Output
```
loop            cycles        insts       stalls    CPI   %stall
independent      7073149   12583365    2840011   0.56   40.2%
1-dep            9706776   13631909    4764087   0.71   49.2%
2-dep           13699238   18874591    7075059   0.73   51.6%
```

### Discussion
- Data dependency can greatly stall CPU pipeline (40.2% -> 49.2%)
- But introducing more dependencies on the same data (i.e. a[i-2] on top of a[i-1]) seem to have less impact compared to the first introduction of dependency, but still non-negligible.