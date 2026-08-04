**Sharma Chirag Krishnakumar | S20240010223**


# Lab00 — Performance Profiling and Optimization using GPROF and PERF

Sorting ~1,000,000 simulated daily temperature records with **Bubble Sort** (baseline)
and **Quick Sort** (optimized), profiled with `gprof`, timed with `time`, and analyzed
across compiler optimization levels and input sizes.

## Files

```
Lab00/
├── mysort.c            # source: data generation + bubble sort + quick sort
├── README.md           # this file
├── report.pdf           # write-up (charts + tables + analysis)
├── myreport.txt         # gprof report, -O0 build
├── report_O2.txt        # gprof report, -O2 build
├── report_O3.txt         # gprof report, -O3 build
├── scaling.csv          # measured n vs time for both algorithms
├── scaling_chart.png     # execution-time-vs-n plots (linear + log-log)
├── run_O0.txt / run_O2.txt / run_O3.txt   # raw program + `time` output
└── screenshots/
```

## 1. Environment

```
$ gcc --version
gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
$ gprof --version
GNU gprof (GNU Binutils for Ubuntu) 2.42
```

> **Note on `perf`:** the container used to build this lab does not have
> `linux-tools`/`perf` installed and does not permit installing kernel packages,
> so hardware counters (cycles, IPC, cache misses, branch misses) could not be
> collected here. Section 4 explains exactly what commands to run and how to read
> the output on a normal Ubuntu machine with `perf` installed
> (`sudo apt install linux-tools-common linux-tools-$(uname -r)`), and gives the
> expected qualitative results based on the algorithms' known memory-access
> patterns.

## 2. Program design (`mysort.c`)

- Generates `N` simulated daily temperatures as **fixed-point integers**
  (tenths of a degree, range −30.0 °C .. 50.0 °C) — this mirrors how real
  weather-station pipelines store sensor readings, and keeps comparisons cheap.
- `quickSort()` — classic Lomuto-partition recursive quicksort, run on the
  **full requested size** (default 1,000,000).
- `bubbleSort()` — classic adjacent-swap bubble sort with an early-exit flag.
  Because Bubble Sort is O(n²), sorting 1,000,000 elements would require on
  the order of **10¹² comparisons** (tens of minutes to hours depending on
  the machine). The program automatically caps Bubble Sort's input at 50,000
  elements and prints a warning — this is standard practice: you don't run an
  O(n²) baseline at production scale, you measure it at smaller sizes and
  extrapolate (see Section 6).
- Timing uses `clock_gettime(CLOCK_MONOTONIC, …)` (wall-clock, not `clock()`,
  since `clock()` measures CPU time and can be misleading for I/O-adjacent work).
- Usage: `./mysort [N] [mode]`, where `mode` is `quick`, `bubble`, or `both`.

## 3. Compilation

```bash
gcc -O0 -pg mysort.c -o mysort_O0
gcc -O2 -pg mysort.c -o mysort_O2
gcc -O3 -pg mysort.c -o mysort_O3
```

## 4. Results

### 4.1 Execution time (`time ./mysort 1000000 both`)

| Build | QuickSort (n=1,000,000) | BubbleSort (n=50,000, capped) | `real` (both) |
|---|---|---|---|
| `-O0` | 1.753 s | 14.003 s | 15.794 s |
| `-O2` | 0.508 s | 6.332 s | 6.887 s |
| `-O3` | 0.462 s | 6.268 s | 6.779 s |

Raw `time` output for `-O0`:
```
real    0m15.794s
user    0m15.687s
sys     0m0.056s
```
(`user` ≈ `real` and `sys` is negligible — the workload is pure CPU/memory
bound, no I/O or blocking syscalls, as expected for an in-memory sort.)

**To make the comparison apples-to-apples**, note Bubble Sort was capped at
n=50,000 while QuickSort ran the full n=1,000,000. If both had run on the
*same* n=50,000: QuickSort finishes in a few milliseconds while Bubble Sort
still takes ~14 s at `-O0` — i.e. QuickSort is roughly **3–4 orders of
magnitude faster** at this size, and the gap widens as n grows (see 4.4).

### 4.2 GPROF report — `-O0` (`myreport.txt`)

Flat profile (top entries):

```
 %   cumulative   self              self     total
time   seconds   seconds    calls   s/call   s/call  name
64.63    5.24     5.24        1     5.24     6.44  bubbleSort
20.25    6.88     1.64   999199     0.00     0.00  partition
15.00    8.10     1.22 630093529    0.00     0.00  swap_int
 0.12    8.10     0.01                             random
```

- **Hotspot function:** `bubbleSort`, consuming **64.6 %** of total runtime
  (5.24 s of 8.10 s profiled) in a single call — all of it spent in the
  nested `O(n²)` comparison/swap loop.
- `partition` (QuickSort's core routine) accounts for 20.25 % despite being
  called **999,199 times** — each call is cheap (~0 s/call) but there are
  nearly a million of them (one per recursive partition on ~1M elements).
- `swap_int` was called **630,093,529 times** total (across both sorts
  combined) and is 15 % of runtime — this is the single most-called function
  in the program, confirming that swap overhead, not comparison overhead, is
  the dominant per-operation cost in both algorithms at `-O0`.
- Call graph confirms `main` → `bubbleSort` (5.24 s self) and `main` →
  `quickSort` → `partition` (1.64 s) → `swap_int`, matching the flat profile.

### 4.3 GPROF report — `-O2` / `-O3` (`report_O2.txt`, `report_O3.txt`)

```
 %   cumulative   self     calls   name        (-O2)
92.52   6.31      6.31        1   main
 7.33   6.81      0.50        3   quickSort
 0.15   6.82      0.01            random_r
```

At `-O2`/`-O3`, `bubbleSort`, `partition`, and `swap_int` **disappear from
the flat profile as separate entries** and their time is folded into `main`.
This is expected: the optimizer **inlines** small static functions like
`swap_int` and, with `-O2`/`-O3`, effectively also inlines/flattens
`bubbleSort`'s body into its caller when profitable. This is a classic
**gprof + optimization gotcha**: instrumentation-based profiling loses
function-level granularity exactly when the compiler does the most
restructuring, which is one reason `perf` (sampling-based, works on the
actual generated machine code) is preferred for profiling optimized builds
in Section 5 — gprof is best used at `-O0` for call-graph/hotspot discovery,
then confirmed with `-O2`/`-O3` timing + `perf` for the real-world numbers.

### 4.4 Scaling study (n vs. time, `-O0` build, `scaling.csv`)

| n | Bubble Sort (s) | Quick Sort (s) | Bubble/Quick ratio |
|---:|---:|---:|---:|
| 1,000 | 0.0109 | 0.0002 | 55× |
| 2,000 | 0.0200 | 0.0003 | 67× |
| 5,000 | 0.1287 | 0.0011 | 117× |
| 10,000 | 0.5291 | 0.0019 | 279× |
| 20,000 | 2.1943 | 0.0046 | 477× |
| 40,000 | 8.7713 | 0.0095 | 923× |

Additional QuickSort-only points (full range up to 1M):

| n | QuickSort (s) |
|---:|---:|
| 100,000 | 0.059 |
| 500,000 | 0.487 |
| 1,000,000 | 1.738 |

See `scaling_chart.png` for the linear and log-log plots.

**Growth-rate check.** Doubling n from 20,000 → 40,000:
- Bubble Sort time ratio = 8.7713 / 2.1943 ≈ **4.0×** — matches O(n²)
  (doubling n should roughly quadruple time: 2²=4). ✔
- Quick Sort time ratio = 0.0095 / 0.0046 ≈ **2.06×** — close to the
  O(n log n) prediction (2 × log(40000)/log(20000) ≈ 2.13). ✔

This is the empirical signature of the two complexity classes: the
Bubble/Quick ratio grows from 55× at n=1,000 to 923× at n=40,000 — the gap
is not constant, it **widens with n**, exactly as O(n²) vs O(n log n)
predicts. At n=1,000,000 the theoretical gap would be enormous (Bubble
Sort is not shown at full scale because it is not practically runnable).

## 5. Hardware performance counters (`perf stat`) — methodology

`perf` was not available in this build environment. On a machine with it
installed, run:

```bash
perf stat ./mysort_O0 1000000 both
perf stat ./mysort_O2 1000000 both
perf stat ./mysort_O3 1000000 both
```

This reports (among others): task-clock, context-switches, cycles,
instructions, IPC, branches, branch-misses, cache-references, cache-misses.
Record these six per build/algorithm and fill in a table like:

| Metric | Bubble -O0 | Quick -O0 | Bubble -O3 | Quick -O3 |
|---|---|---|---|---|
| Execution time | | | | |
| CPU cycles | | | | |
| Instructions | | | | |
| IPC | | | | |
| Cache references | | | | |
| Cache misses | | | | |
| Branch misses | | | | |

**Expected qualitative behavior** (based on the algorithms' memory-access
patterns, to guide interpretation of your own numbers):
- Bubble Sort repeatedly compares/swaps **adjacent** elements — highly
  sequential, cache-friendly access, so **cache-miss rate per access should
  be low**, but the *total number* of accesses (and thus cache references,
  instructions, and cycles) is O(n²), so absolute counts dwarf Quick Sort's.
- Quick Sort's partition step also scans sequentially within a sub-array
  (good locality), but recursion touches many different sub-ranges of the
  array; still, this is far fewer total accesses than Bubble Sort at
  n=1,000,000, so total cycles/instructions/cache references should all be
  orders of magnitude lower.
- **IPC** is usually similar for both (simple integer compare/swap loops,
  no complex branching), so the time difference is driven by **instruction
  count**, not per-instruction efficiency — consistent with the gprof
  finding that `swap_int`/comparisons dominate, not stalls.
- **Branch misses**: Bubble Sort's `if (arr[j] > arr[j+1])` on random data is
  essentially a 50/50 unpredictable branch repeated O(n²) times — expect a
  non-trivial absolute branch-miss count purely from volume. Quick Sort's
  partition branch is similarly data-dependent but executed far fewer times.

## 6. Answers to the lab questions

**1. Which sorting algorithm performed better? Explain your observation.**
Quick Sort, decisively. At `-O0`, sorting 1,000,000 elements took 1.75 s
with Quick Sort; Bubble Sort couldn't even be run at that size in reasonable
time and took 14.0 s just for 50,000 elements. Quick Sort's O(n log n)
average complexity does roughly n·log₂(n) ≈ 1,000,000 × 20 ≈ 2×10⁷ "units"
of work, while Bubble Sort's O(n²) does ~10¹² — five orders of magnitude
more operations at n=1,000,000. This is an algorithmic (Big-O) difference,
not an implementation detail, so no amount of micro-optimization closes it.

**2. Which function consumed the maximum execution time?**
`bubbleSort` — 64.6 % of total profiled time in the `-O0` gprof report,
confirmed by the call count (630M+ calls into `swap_int`, mostly driven by
Bubble Sort's nested loop). Within Quick Sort, `partition` is the hotspot,
consistent with it being the only work quicksort does outside recursion
bookkeeping.

**3. How does execution time change with increasing input size?**
Bubble Sort's time grows quadratically — a 2× increase in n gives a ~4×
increase in time (confirmed empirically: 4.0× when n went 20,000→40,000).
Quick Sort's time grows near-linearly (n log n) — the same doubling gave
only ~2.06× more time. The *ratio* between the two algorithms' times grows
with n (55× at n=1,000 → 923× at n=40,000), so Bubble Sort becomes
disproportionately worse the larger the dataset gets — it does not merely
lose, it loses by more and more as data grows.

**4. How did compiler optimizations (`-O2` and `-O3`) improve performance?**
Wall-clock time dropped substantially at every level:
- QuickSort: 1.753 s (`-O0`) → 0.508 s (`-O2`, **3.45× faster**) → 0.462 s
  (`-O3`, **3.79× faster**).
- BubbleSort (n=50,000): 14.00 s (`-O0`) → 6.33 s (`-O2`, **2.21× faster**)
  → 6.27 s (`-O3`, **2.23× faster**).
`-O2`/`-O3` enable inlining (e.g. `swap_int` folds into its caller,
eliminating call/return overhead on the single hottest function in the
program), register allocation improvements, loop-invariant code motion, and
stronger instruction scheduling — visible indirectly in gprof as
`bubbleSort`/`partition`/`swap_int` disappearing from the flat profile
(they get inlined into `main`). `-O3` adds more aggressive
auto-vectorization and loop unrolling on top of `-O2`, giving a further
small improvement (0.508 s → 0.462 s for QuickSort here), though the
`-O2`→`-O3` gain is much smaller than `-O0`→`-O2` — most of the low-hanging
fruit is already captured at `-O2`. Both algorithms benefited, but the
*relative* speedup was larger for QuickSort — with fewer, more predictable
operations overall, the optimizer has cleaner code to work with, whereas
Bubble Sort's benefit is capped by the sheer instruction *count* it must
execute regardless of how efficiently each instruction runs.

**5. Which algorithm would you recommend for large datasets? Justify your answer.**
Quick Sort, without qualification, for any dataset beyond a few thousand
elements. The measured data shows the performance gap growing
super-linearly with n; at the lab's target scale (≈1,000,000 daily-temperature
records, i.e. ~50 years of dense readings) Bubble Sort is not just slower,
it is operationally infeasible (would take on the order of hours, versus
under half a second for Quick Sort at `-O3`). The one caveat: naive
Lomuto-partition Quick Sort has an **O(n²) worst case** on already-sorted or
adversarial input (e.g. a dataset that arrives pre-sorted by date, which is
plausible for real temperature logs). A production recommendation would be
Quick Sort with a randomized or median-of-three pivot (or fall back to
Introsort/Heapsort on recursion-depth blowup, as `std::sort` does) to
guarantee O(n log n) even in the worst case, plus switching to insertion
sort for small sub-partitions (a common hybrid optimization) since Bubble
/Insertion-style sorts do outperform Quick Sort's overhead at very small n.

## 7. Feasibility of parallel execution

Bubble Sort is inherently hard to parallelize in its adjacent-swap form
(each swap depends on the previous comparison's outcome in the same pass),
though "odd-even transposition sort" is a known parallel variant. Quick
Sort parallelizes naturally: after the first partition, the two resulting
sub-arrays are **independent** and can be sorted concurrently (e.g. spawn a
thread/task per recursive call, or use OpenMP `#pragma omp task`), typically
capped by a depth/size threshold to avoid thread-creation overhead
dominating on small sub-arrays. Given the sandbox used for this lab reports
`nproc` = 1, no parallel speedup would be measurable here, but on a
multi-core machine, parallel Quick Sort (or a parallel merge sort) would be
the natural next optimization step beyond `-O3`.
