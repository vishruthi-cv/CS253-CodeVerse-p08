# System Architecture & Interface Contract (`IPercentileTracker`)

## 1. Architectural Overview

This repository separates the **Domain Application Layer** (`MonitoringApp`) from the **Algorithmic Data Structure Layer** (`structA` and `structB`) using a polymorphic abstract interface (`include/IPercentileTracker.h`).

- **Application Layer (`app/`, `include/MonitoringApp.h`):** Consumes live telemetry streams (`recordMetric`), processes late-arriving event retractions (`retractMetric`), slides time windows, and evaluates real-time SLA breaches (`checkSLA`) strictly against `IPercentileTracker` without coupling to internal heap or Fenwick tree mechanics.
- **Data Structure A (`src/structA/`, wrapped by `app/src/TrackerAAdapter.h`):** Dual-Heap + Hash-Map Lazy Deletion approach ($O(\log N)$ amortized insert/erase/update per active quantile engine, $O(1)$ percentile peek).
- **Data Structure B (`src/structB/include/PercentileTrackerB.h`):** Coordinate-Compressed Fenwick Tree (Binary Indexed Tree) + ID Table ($O(\log M)$ insert/erase/update, $O(\log M)$ order-statistic binary lifting query).

## 2. Core Interface Specification (`IPercentileTracker`)

Every concrete tracker (`TrackerAAdapter` for `structA` and `PercentileTrackerB` for `structB`) inherits from `IPercentileTracker` and implements the following contract:

| Method | Signature | Complexity Target | Behavioral Contract |
| :--- | :--- | :--- | :--- |
| **Insert** | `virtual void insert(int64_t id, double value) = 0;` | $O(\log N)$ | Associates unique event `id` with latency `value` and adds it to the active window. |
| **Erase** | `virtual bool erase(int64_t id) = 0;` | $O(\log N)$ | Removes the active element with `id`. Returns `true` if found and removed, `false` otherwise. |
| **Update** | `virtual bool update(int64_t id, double newValue) = 0;` | $O(\log N)$ | Updates the latency of an active `id` to `newValue`. Returns `true` if `id` is active, `false` otherwise. |
| **Get Percentile** | `virtual double getPercentile(double p) const = 0;` | $O(1)$ to $O(\log N)$ | Returns the $p$-th percentile ($0.0 < p \le 100.0$) of all active elements using nearest-rank lower middle. |
| **Get Median** | `virtual double getMedian() const = 0;` | $O(1)$ to $O(\log N)$ | Equivalent to `getPercentile(50.0)`. |
| **Size** | `virtual size_t size() const = 0;` | $O(1)$ | Returns the exact count of currently active (non-erased) elements. |
| **Empty** | `virtual bool empty() const = 0;` | $O(1)$ | Returns `true` iff `size() == 0`. |
| **Clear** | `virtual void clear() = 0;` | $O(N)$ | Resets the tracker to an empty state (`size() == 0`). |
| **Engine Name** | `virtual std::string getEngineName() const = 0;` | $O(1)$ | Returns the human-readable identifier of the underlying engine (e.g., `"StructA-DualHeap"`). |

## 3. Edge Case Handling & Invariants (Strict Contract)

Both `structA` and `structB` must satisfy every invariant checked by `oracle/` and produce identical outputs across all edge cases:

1. **Empty Window Rule (`size() == 0`):**
   - Calling `getPercentile(p)` or `getMedian()` on an empty window **must not** throw an exception, dereference empty containers, or return `NaN`.
   - It **must strictly return `-1.0`** (formatted as `-1.000000` in `--eval` mode). `MonitoringApp::checkSLA()` checks `if (val < 0.0)` and skips alerting until active data exists.

2. **Nearest-Rank Lower-Middle Percentile Rule ($N > 0$):**
   - For any percentile $p \in (0.0, 100.0]$ over $N$ sorted active elements $v_1 \le v_2 \le \dots \le v_N$, the 1-indexed rank is strictly:
     $$k = \left\lceil \frac{p \cdot N}{100} \right\rceil$$
     (corresponding to 0-based sorted index $\text{idx} = k - 1$, clamped to $[0, N - 1]$).
   - **Median ($p = 50.0$) when $N$ is even:** Uses the **lower-middle** element $k = N / 2$ (0-based index $N/2 - 1$), **not** the arithmetic mean of the two middle values.

3. **Duplicate Metric Values:**
   - Multiple distinct `id`s may share the exact same `double` value (e.g., `(id=101, val=45.0)` and `(id=102, val=45.0)`).
   - Calling `erase(101)` must remove **only** that single instance and leave `id=102` intact in the active window.

4. **Invalid, Duplicate, or Re-inserted IDs:**
   - Calling `erase(id)` or `update(id, newValue)` for an `id` that never existed or was already erased must be a **safe no-op** that leaves `size()` and all order statistics unchanged and returns `false`.
   - Re-inserting an `id` that is already active cleanly replaces/updates that entry without inflating `size()`.

5. **Out-of-Range Percentile ($p \le 0.0$ or $p > 100.0$):**
   - Queries with $p \le 0.0$ or $p > 100.0$ throw `std::invalid_argument` consistently across both structures.

6. **Sliding Window Synchronization (`--window W`):**
   - When a positive sliding window capacity $W$ is active, inserting a new element when `size() == W` evicts the oldest active element in insertion order so both structures maintain identical active sets.

## 4. Directory Layout Convention

```text
├── include/               # Public abstract contracts & domain application headers
│   ├── IPercentileTracker.h
│   └── MonitoringApp.h
├── src/                   # Concrete data structure implementations
│   ├── structA/           # Dual-Heap + Lazy Deletion
│   └── structB/           # Coordinate-Compressed Fenwick Tree
├── app/                   # Application entry point, CLI eval mode & adapters
├── oracle/                # Reference differential oracle & property tests
├── tests/                 # Correctness verification & latency/throughput benchmarks
├── scripts/               # Automation & benchmark runners
├── docs/                  # Formal proofs, D3 derivations & architecture notes
└── INTERFACE.md           # This architectural contract
