# System Architecture & Interface Contract (`IPercentileTracker`)

## 1. Architectural Overview
This repository separates the **Domain Application Layer** (`MonitoringApp`) from the **Algorithmic Data Structure Layer** (`Struct_A` and `Struct_B`) using a polymorphic abstract interface (`include/IPercentileTracker.h`).

- **Application Layer (`app/`, `include/MonitoringApp.h`):** Consumes live telemetry streams (`recordMetric`), processes late-arriving event retractions (`retractMetric`), slides time windows, and evaluates real-time SLA breaches (`checkSLA`) strictly against `IPercentileTracker` without coupling to internal heap or Fenwick tree mechanics.
- **Data Structure A (`src/Struct_A/`):** Dual-Heap + Hash-Map Lazy Deletion approach (`O(log N)` amortized insert/retract, `O(1)` median/percentile peek).
- **Data Structure B (`src/Struct_B/`):** Coordinate-Compressed Fenwick Tree (Binary Indexed Tree) + ID Table (`O(log M)` insert/retract, `O(log M)` order-statistic binary lifting query).

---

## 2. Core Interface Specification (`IPercentileTracker`)

Every concrete tracker (`Struct_A` and `Struct_B`) must inherit from `IPercentileTracker` and implement the following methods:

| Method | Signature | Complexity Target | Behavioral Contract |
| :--- | :--- | :--- | :--- |
| **Insert** | `virtual void insert(uint64_t id, double value) = 0;` | `O(log N)` | Associates unique event `id` with latency `value` and adds it to the active window. |
| **Retract** | `virtual bool retract(uint64_t id) = 0;` | `O(log N)` | Removes the active element with `id`. Returns `true` if found and removed, `false` otherwise. |
| **Get Percentile** | `virtual double getPercentile(double p) const = 0;` | `O(1)` to `O(log N)` | Returns the `p`-th percentile (`0.0 <= p <= 100.0`) of all active elements. |
| **Get Median** | `virtual double getMedian() const = 0;` | `O(1)` to `O(log N)` | Equivalent to `getPercentile(50.0)`. |
| **Size** | `virtual size_t size() const = 0;` | `O(1)` | Returns the exact count of currently active (non-retracted) elements. |
| **Empty** | `virtual bool empty() const = 0;` | `O(1)` | Returns `true` iff `size() == 0`. |

---

## 3. Edge Case Handling & Constraints (Strict Contract)

Both `Struct_A` and `Struct_B` must produce identical outputs across all edge cases:

1. **Empty Window Rule (`size() == 0`):**
   - Calling `getPercentile(p)` or `getMedian()` on an empty window **must not** throw an exception, dereference null/empty containers, or return `NaN`.
   - It **must strictly return `-1.0`** as a sentinel value. `MonitoringApp::checkSLA()` checks `if (val < 0.0)` and safely skips alerting until active data exists.

2. **Even Element Count (`size() % 2 == 0`):**
   - For median (`p == 50.0`) when `N` is even and `N > 0`, the result **must** be the arithmetic mean of the two middle sorted elements:
     $$\text{median} = \frac{v_{N/2 - 1} + v_{N/2}}{2.0}$$
     *(0-indexed sorted order)*.
   - For general percentiles (`0.0 <= p <= 100.0`), the target 0-based rank is computed consistently via nearest-rank / linear interpolation (`idx = static_cast<size_t>(std::ceil((p / 100.0) * N)) - 1`, clamped to `[0, N - 1]`).

3. **Duplicate Metric Values:**
   - Multiple distinct `id`s may share the exact same `double value` (e.g., `(id=101, val=45.0)` and `(id=102, val=45.0)`).
   - Retracting `id=101` must remove **only** that single instance and leave `id=102` intact in the active window.

4. **Invalid or Duplicate Retractions:**
   - Calling `retract(id)` for an `id` that never existed or was already retracted must be a **safe no-op** that leaves `size()` and all order statistics unchanged and returns `false`.
   - Inserting an `id` that is already active must either overwrite/update cleanly or be rejected without corrupting internal counts.

5. **Out-of-Range Percentile (`p < 0.0` or `p > 100.0`):**
   - Queries with `p < 0.0` or `p > 100.0` must either clamp `p` to `[0.0, 100.0]` or throw `std::invalid_argument` consistently across both structures (default: throw `std::invalid_argument`).

---

## 4. Directory Layout Convention

```text
├── include/               # Public abstract contracts & domain application headers
│   ├── IPercentileTracker.h
│   └── MonitoringApp.h
├── src/                   # Concrete data structure implementations
│   ├── Struct_A/          # Dual-Heap + Lazy Deletion
│   └── Struct_B/          # Coordinate-Compressed Fenwick Tree
├── app/                   # Application entry point & telemetry simulation
├── tests/                 # Correctness verification & latency/throughput benchmarks
├── docs/                  # Formal proofs, D3 derivations & architecture notes
└── INTERFACE.md           # This architectural contract
