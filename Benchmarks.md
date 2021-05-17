# Benchmark Results

## System Specification

### Hardware

```txt
Memory:    RAM: total: 30.62 GiB
CPU:       Topology: 16-Core (4-Die) model: AMD EPYC (with IBPB) bits: 64 type: MCP MCM arch: Zen rev: 2
           L2 cache: 8192 KiB
           flags: avx avx2 lm nx pae sse sse2 sse3 sse4_1 sse4_2 sse4a ssse3 bogomips: 79849
           Speed: 2495 MHz min/max: N/A Core speeds (MHz): 1: 2495 2: 2495 3: 2495 4: 2495 5: 2495 6: 2495 7: 2495
           8: 2495 9: 2495 10: 2495 11: 2495 12: 2495 13: 2495 14: 2495 15: 2495 16: 2495
```

### Software

```txt
System:    Host: ubuntu-32gb-nbg1-1 Kernel: 5.4.0-72-generic x86_64 bits: 64 compiler: gcc v: 9.3.0 Console: N/A
           dm: N/A Distro: Ubuntu 20.04.2 LTS (Focal Fossa)
Compiler:  Ubuntu clang version 11.0.0-2~ubuntu20.04.1
           Target: x86_64-pc-linux-gnu
           Thread model: posix
```

## Results

Execution times are the average of processing an image with dimensions **43680px x 4160px** with **100 times** iterations.

### pthread

![pthread benchmark diagram](/benchmark_diagrams/pthread.png)

### OpenMP

![OpenMP benchmark diagram](/benchmark_diagrams/openmp.png)

### pthread vs. OpenMP

![pthread vs. OpenMP 1 workers benchmark diagram](/benchmark_diagrams/01w.png)

![pthread vs. OpenMP 2 workers benchmark diagram](/benchmark_diagrams/02w.png)

![pthread vs. OpenMP 3 workers benchmark diagram](/benchmark_diagrams/03w.png)

![pthread vs. OpenMP 4 workers benchmark diagram](/benchmark_diagrams/04w.png)

![pthread vs. OpenMP 5 workers benchmark diagram](/benchmark_diagrams/05w.png)

![pthread vs. OpenMP 6 workers benchmark diagram](/benchmark_diagrams/06w.png)

![pthread vs. OpenMP 7 workers benchmark diagram](/benchmark_diagrams/07w.png)

![pthread vs. OpenMP 8 workers benchmark diagram](/benchmark_diagrams/08w.png)

![pthread vs. OpenMP 9 workers benchmark diagram](/benchmark_diagrams/09w.png)

![pthread vs. OpenMP 10 workers benchmark diagram](/benchmark_diagrams/10w.png)

![pthread vs. OpenMP 11 workers benchmark diagram](/benchmark_diagrams/11w.png)

![pthread vs. OpenMP 12 workers benchmark diagram](/benchmark_diagrams/12w.png)

![pthread vs. OpenMP 13 workers benchmark diagram](/benchmark_diagrams/13w.png)

![pthread vs. OpenMP 14 workers benchmark diagram](/benchmark_diagrams/14w.png)

![pthread vs. OpenMP 15 workers benchmark diagram](/benchmark_diagrams/15w.png)

![pthread vs. OpenMP 16 workers benchmark diagram](/benchmark_diagrams/16w.png)

![pthread vs. OpenMP 17 workers benchmark diagram](/benchmark_diagrams/17w.png)

![pthread vs. OpenMP 18 workers benchmark diagram](/benchmark_diagrams/18w.png)

![pthread vs. OpenMP 19 workers benchmark diagram](/benchmark_diagrams/19w.png)

![pthread vs. OpenMP 20 workers benchmark diagram](/benchmark_diagrams/20w.png)

![pthread vs. OpenMP 21 workers benchmark diagram](/benchmark_diagrams/21w.png)

![pthread vs. OpenMP 22 workers benchmark diagram](/benchmark_diagrams/22w.png)

![pthread vs. OpenMP 23 workers benchmark diagram](/benchmark_diagrams/23w.png)

![pthread vs. OpenMP 24 workers benchmark diagram](/benchmark_diagrams/24w.png)

![pthread vs. OpenMP 25 workers benchmark diagram](/benchmark_diagrams/25w.png)

![pthread vs. OpenMP 26 workers benchmark diagram](/benchmark_diagrams/26w.png)

![pthread vs. OpenMP 27 workers benchmark diagram](/benchmark_diagrams/27w.png)

![pthread vs. OpenMP 28 workers benchmark diagram](/benchmark_diagrams/28w.png)

![pthread vs. OpenMP 29 workers benchmark diagram](/benchmark_diagrams/29w.png)

![pthread vs. OpenMP 30 workers benchmark diagram](/benchmark_diagrams/30w.png)

![pthread vs. OpenMP 31 workers benchmark diagram](/benchmark_diagrams/31w.png)

![pthread vs. OpenMP 32 workers benchmark diagram](/benchmark_diagrams/32w.png)
