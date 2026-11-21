# Running-median-with-retractions-

## Seeded oracle differential test

`oracle/oracle_diff.cpp` compares Structure A at p50 and p90, Structure B at p50 and p90, and a sorted-vector oracle. It runs one million seeded insertion steps with a window of 1,000 and values from 0 through 9,999. Each step may also retract one randomly selected active reading. Expiry is applied at the window boundary. All results and live counts are compared after each operation; Structure A's full consistency checker runs every 256 insertions for both percentiles. If a mismatch occurs, the program prints the seed and uses chunk-removal shrinking to produce a 1-minimal failing sequence (no single remaining operation can be removed while preserving the failure).

Seed: `20261121`.

Build from the repository root with:

```sh
g++ -std=c++17 -O2 -Iinclude -Isrc/structA/include -Isrc/structB/include src/structA/src/ExpiryQueue.cpp src/structA/src/HashTable.cpp src/structA/src/MaxHeap.cpp src/structA/src/MedianEngine.cpp src/structA/src/MinHeap.cpp src/structB/src/FenwickTree.cpp src/structB/src/IdTable.cpp src/structB/src/PercentileEngine.cpp src/structB/src/PercentileTrackerB.cpp src/structB/src/SortedValuetable.cpp oracle/oracle_diff.cpp -o oracle_diff
```

Run in Windows PowerShell with:

```sh
.\oracle_diff.exe
```

On Linux or macOS, run with:

```sh
./oracle_diff
```

A passing run prints the seed, step count, and window, followed by `ALL TESTS PASSED`.
