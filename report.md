## Predicted cost model (written before measuring)

Notation: w = window size (live readings), U = distinct values, r = fraction of readings retracted early, θ = purge threshold (purge when dead entries exceed θ × live entries; we use θ = 1). Both expiry and retraction are `retract(id)` calls.

### Structure A (two heaps, lazy deletion, purge)
- Time per operation: about c1·log2(w) for heap push, sift and rebalancing, plus an amortised purge term.
- Purge term: a purge costs about (1+θ)·w steps, and at least θ·w retractions are needed to create that many dead entries since the last purge. Amortised cost per retraction is therefore (1+θ)/θ = O(1) for fixed θ. Total: O(log w) amortised.
- Memory: heap entries stay below about (1+θ)·w, so about 2·w for θ = 1, plus a pending table of at most about θ·w entries. Without a purge, dead entries could grow without bound under an adversarial pattern.
- Effect of r: more retractions mean more dead entries and more purges, so time per operation should rise slowly with r, bounded by the O(1/θ) purge term.

### Structure B: Fenwick tree over compressed values

**Representation.** All distinct stream values are sorted once into `SortedValueTable`, giving each value a rank index from `1` to `U`. A 1-indexed `FenwickTree` over `1..U` tracks the frequency count of active readings at each rank. Because a retraction only gives a reading's `id`, a custom chained hash table (`IdTable`) maps each live `id` to its value. `PercentileEngine` pairs the sorted table with the Fenwick tree, and `PercentileTrackerB` wraps both together with `IdTable`.

**Operations and cost.**

| Operation | How it works | Time |
|---|---|---|
| `insert(id, v)` | Insert `(id, v)` into `IdTable`, binary search `v` in `SortedValueTable`, call `add(pos, +1)` | $O(\log U)$ |
| `retract(id)` | Look up `v` in `IdTable`, binary search `pos`, call `add(pos, -1)`, erase `id` | $O(\log U)$ |
| `kth(k)` | Run binary-lifting descent (`findKth(k)`), then return `valueAt(pos)` | $O(\log U)$ |
| `median()` / `percentile(p)` | Compute $k = \lceil p \cdot \text{total} \rceil$, then call `kth(k)` | $O(\log U)$ |

`IdTable` uses `100003` buckets for up to $10^5$ live readings, keeping hash lookups $O(1)$ expected. Total memory is $O(U + w)$.

**Invariants.**
1. `SortedValueTable` stores distinct values in strictly increasing order.
2. Each Fenwick entry `F[i]` equals the sum of active counts in the range $(i - \text{lowbit}(i),\, i]$.
3. Every frequency count is non-negative ($\ge 0$).
4. `total` equals both the sum of all active counts in the tree and the number of entries in `IdTable`.

**Design decisions.**
* **Offline coordinate compression instead of direct value indexing:** Sensor readings can be floating-point numbers or large integers, so indexing an array directly by value is impossible. Sorting the distinct values upfront keeps the Fenwick tree compact at size $U$ rather than spanning the full numerical range.
* **Fenwick tree instead of an order-statistic balanced BST:** While a balanced BST avoids pre-knowing $U$, a Fenwick tree is stored in a flat contiguous array with much smaller constant factors, lower memory overhead per element, and simpler query logic.
* **Custom chained hash table (`IdTable`) instead of `std::unordered_map`:** Project rules prohibit using standard library associative containers inside the structures under study, so `IdTable` was implemented from scratch using separate chaining with a prime bucket count (`100003`).

**Link to derivation.** The correctness and $O(\log U)$ bound of `findKth` are proved in Derivation D2.

### Oracle (reference only)
- Sorted array: binary search O(log w), but shifting elements makes each insert and delete O(w). Expected to grow roughly linearly with w.

### Numbers to check
| Quantity | Prediction |
|---|---|
| A, time per op, w from 10^2 to 10^5 | log2 w goes from 6.6 to 16.6, so at least about 2.5x slower |
| B, time per op, same w range, fixed U | roughly flat (ratio near 1) |
| A, memory vs r | between w and about 2w entries; no unbounded growth |
| B, memory vs r | flat |
| Oracle, time per op, same w range | roughly linear in w |

### What could make it wrong
If the measured growth is larger than predicted, possible causes are cache effects and memory allocation. We will not claim cache effects without profiler output.