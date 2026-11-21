# Project 8: Running Median with Retractions

Team 8 CodeVerse - CS 253 / MA 253, Autumn 2026

## Team and roles

| Name | Roll no. | GitHub | Role | Module folder | Derivation |
|------|----------|--------|------|---------------|------------|
| Vishruthi | 250001079| @vishruthi-cv | Structure A owner | src/Struct_A/ | D1 |
| Neharika | 250001073 | @TODO | Structure B owner | src/Struct_B/ | D2 |
| Dwithi | 250001075 | @TODO | Analyst | bench/, scripts/ | D6 |
| Kalyani | 250001077 | @KALYANI-cse250001077 | Architect & Integration Engineer | include/, app/src/ | D3 |
| Anushka | TODO | @TODO | TODO | TODO | D3 |

## Summary

The problem is to keep a running median (and the p50 and p90 percentiles) of a stream of readings where earlier readings can be retracted, meaning removed again by their id. Structure A uses two heaps (a max heap and a min heap) with a hash table and an expiry queue, and it removes retracted values lazily. Structure B uses a Fenwick tree over the compressed set of values and finds the k-th smallest value by binary-lifting descent.

## Requirements

- OS: Windows 10 or 11 with MSYS2 (also expected to work on Ubuntu 22.04, TODO confirm)
- Language: C++17
- Compiler: g++ 16.2.0 (MSYS2), flags TODO (for example -O2 -std=c++17)
- Libraries: standard library only in the program. Python 3 with matplotlib, pandas and numpy is used only for plots and data generation.

## Build

TODO single command, for example:

    make

## Quick start

TODO command for a small example, for example:

    ./bin/app --input examples/small.txt

Expected output:

    TODO paste the real output here

## Tests

TODO command, for example:

    make test

The differential test compares Structure A and Structure B against a sorted-vector oracle for p50 and p90, using seed TODO and TODO operations. The invariant checks include MedianEngine::checkConsistency() for Structure A.

A passing run ends with:

    TODO paste the real last line, for example: ALL TESTS PASSED

## Evaluation interface (hidden tests)

TODO use the exact commands from the published format for Project 8, for example:

    ./bin/run_ops --structure A < ops.txt > out.txt
    ./bin/run_ops --structure B < ops.txt > out.txt

Input format: TODO describe one line of input.
Output format: TODO describe one line of output.
Example: TODO one small input and its output.

Our program follows the format published for Project 8.

## Benchmarks and figures

Reruns every experiment and regenerates every figure:

    ./scripts/run_all.sh

Takes about TODO minutes. It writes TODO results/raw and results/figures. The benchmark also checks that A and B agree on every configuration and prints "A and B AGREE".

## Repository map

- Application/ - the monitoring application (include/ and src/main.cpp), with the common interface IPercentileTracker.h
- src/Struct_A/ - Structure A: hash table, min and max heaps, expiry queue, MedianEngine
- src/Struct_B/ - Structure B: Fenwick tree, compressed value table, id table, PercentileTrackerB
- bench/ - benchmark program (bench.cpp)
- adversary/ - adversarial input generators for dead entries in A and the compressed domain in B (TODO remove this line if the folder is not there)
- scripts/ - run_all.sh and plot.py
- docs/derivations/ - derivations D1 to D6
- REPORT.md - full analysis
- TOOLS.md - tools and AI use declaration

## Results at a glance

| Experiment | Structure A | Structure B |
|------------|-------------|-------------|
| TODO time per operation | TODO | TODO |
| TODO memory | TODO | TODO |
| TODO worst case input | TODO | TODO |

See REPORT.md for the full analysis and the predicted cost model.

## Known limitations

- TODO write honestly what is incomplete or slow, for example inputs where Structure A builds up many dead entries, or value ranges Structure B does not support.
- TODO the static linking flag (-static) was needed on our Windows machine to avoid a crash, and it may not be needed on Linux.

## Tools and references

See TOOLS.md and REPORT.md, section References.

Main papers and sources: TODO list the references you actually used.
