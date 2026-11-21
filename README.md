# Project 8: Running Median with Retractions

Team 8 CodeVerse - CS 253 / MA 253, Autumn 2026

## Team and roles

| Name | Roll no. | GitHub | Role | Module folder | Derivation |
|------|----------|--------|------|---------------|------------|
| Vishruthi | 250001079 | @vishruthi-cv | Structure A owner | src/structA/ | D1 |
| Neharika | 250001073 | @Neharika1510 | Structure B owner | src/structB/ | D2 |
| Kalyani | 250001077 | @KALYANI-cse250001077 | Architect | include/, app/ | D3 |
| Anushka | 250001080 | @ANUSHKA-Cse250001080 | Oracle owner | oracle/ | D4 |
| Sahana | 250001062 | - | Adversary | adversary/ | D5 |
| Tummala Dwithi | 250001075 | @dwithi456 | Analyst | bench/, scripts/, results/ | D6 |

## Summary

The program keeps the running median, and other percentiles such as p90, of the latest w readings of a stream. A reading can also be retracted (removed again by its id) at any time, and old readings expire from the window. Structure A uses two heaps (a max-heap and a min-heap) with a hash table and an expiry queue; removals are lazy, and a purge rebuilds the heaps when dead entries outnumber live ones. Structure B uses a Fenwick tree over the sorted distinct values and finds the k-th smallest value by binary-lifting descent. An oracle (a sorted array) checks both, and a benchmark compares their time and memory against a cost model written before measuring.

## Requirements

- OS: Windows 10 or 11 with MSYS2 / Git Bash (the machine used for all measurements). Linux with g++ is expected to work too, but we only tested a Linux build, not the full benchmark.
- Language: C++17, standard library only.
- Compiler: g++ 16.2.0 (MSYS2), flags `-std=c++17 -O2`.
- Python 3 with matplotlib, pandas and numpy, only for the benchmark figures.
- `make` (on MSYS2 it may be called `mingw32-make`).

## Build

One command from the repository root:

    make

This builds `build/app`, `build/bench`, `build/oracle_diff` and `build/adversary`. `make clean` deletes the `build/` folder.

## Quick start

    ./build/app

Expected output:

    Starting Dashboard Mode...

    ======================================================
     TELEMETRY ENGINE: StructB-Fenwick
     Active Window Size : 3 events
     Processed / Retract: 3 / 0
     SLA Violations     : 0
     P50 (Median)       : 52.80 ms
     P95 Latency        : 110.50 ms
     P99 Latency        : 110.50 ms
    ======================================================

## Tests

    make test

This runs three `bench check` runs (A and B must give the same answer at every step) and then the differential test against the oracle: `oracle/oracle_diff.cpp` runs 1,000,000 seeded operations (seed 20261121, window 1000), compares the oracle, A and B for p50 and p90 at every step, checks Structure A's invariants with `MedianEngine::checkConsistency()`, and shrinks a failing sequence to a minimal one.

A passing run ends with:

    ALL TESTS PASSED

## Evaluation interface (hidden tests)

    ./build/app --eval --structure A < ops.txt > out.txt
    ./build/app --eval --structure B < ops.txt > out.txt

Input format, one operation per line (ids are 1, 2, 3, ... in the order of the INSERT lines):

    INSERT <value>        add a reading
    RETRACT <id>          remove the reading with this id
    UPDATE <id> <value>   change the value of an existing reading
    QUERY <p>             print the p-th percentile (p = 50 is the median)

Output format: one line per QUERY, the answer with six decimals; `-1.000000` when the window is empty. The percentile is the nearest rank, k = ceil(p * n / 100), so the lower middle value for an even count.

Example input (`ops.txt`):

    INSERT 5
    INSERT 1
    INSERT 9
    INSERT 3
    INSERT 7
    QUERY 50
    QUERY 90
    RETRACT 2
    QUERY 50

Example output:

    5.000000
    9.000000
    5.000000

## Benchmarks and figures

Reruns every experiment and regenerates every figure:

    ./scripts/run_all.sh

It takes a few minutes (add the word `quick` for a shorter test run). It writes the raw numbers to `results/raw/bench.csv` (seed 1, repetition k uses seed 1 + k) and the figures to `results/figures/`. The benchmark first checks that A and B agree and prints "A and B AGREE". It needs bash and Python 3 with matplotlib, pandas and numpy.

## Repository map

- `include/` - the common interface `IPercentileTracker.h` and the monitoring application header
- `app/` - the monitoring application and the evaluation mode
- `src/structA/` - Structure A: hash table, min and max heaps, expiry queue, `MedianEngine`
- `src/structB/` - Structure B: Fenwick tree, sorted value table, id table, `PercentileTrackerB`
- `oracle/` - the sorted-array oracle, the small comparison test and the 10^6-operation differential test
- `adversary/` - generators for adversarial inputs (dead entries in A, a large value domain for B)
- `bench/` - the benchmark program
- `scripts/` - `run_all.sh` and `plot.py`
- `results/` - raw CSV and figures
- `docs/derivations/` - derivations D1 to D6
- `INTERFACE.md`, `REPORT.md`, `REPORT.pdf`, `TOOLS.md`

## Results at a glance

All numbers are means of five timed runs on one machine (Intel i7-13620H, 16 GB, Windows, g++ 16.2.0 -O2), N = 10^6 readings, p = 50.

| Experiment | Structure A | Structure B |
|------------|-------------|-------------|
| Time per step as w grows from 100 to 100,000 | 146 to 207 ns | about 261 ns, flat |
| Time per step as U grows from 1,000 to 1,000,000 | 177 to 168 ns, flat | 148 to 436 ns |
| Time per step as the retraction rate grows from 0% to 50% | 156 to 208 ns | 261 to 292 ns |
| Memory as w grows from 100 to 100,000 | 0.1 to 15 MB | about 7.5 MB, flat |
| Memory as U grows from 1,000 to 1,000,000 | about 1.9 MB, flat | 5.6 to 25 MB |
| Worst-case input | heaps stay at most 2w entries (D5); not measured under the adversary generators | memory grows with U (25 MB at U = 10^6) |

See REPORT.md for the figures, the predicted cost model and the gaps between prediction and measurement.

## Known limitations

- No profiler was run, so the explanations of the gaps (cache effects, fixed per-step costs, a fixed memory floor in B) are hypotheses.
- The oracle's running time was not measured, and we did not implement or measure the Härdle-Steiger method.
- Memory is measured once per setting (on the warm-up run), so it has no error bars.
- Structure A uses a fixed threshold of 1 (purge when dead entries outnumber live ones), and one `MedianEngine` serves one fixed percentile.
- Structure B needs all possible values in advance, and its id table has a fixed 100,003 buckets, so lookups slow down when more than about 10^5 readings are live.
- The static linking flag (`-static`) was needed on our Windows machine to avoid a crash; it may not be needed on Linux.

## Tools and references

AI and other tools are declared in TOOLS.md. References are listed in REPORT.md, section References: Williams (1964), Floyd (1964), Fenwick (1994), Hardle and Steiger (1995), Cormen et al. (2009), Sedgewick and Wayne (2011).
