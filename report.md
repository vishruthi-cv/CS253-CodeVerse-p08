## Predicted cost model (written before measuring)

Notation: w = window size (live readings), U = distinct values, r = fraction of readings retracted early, θ = purge threshold (purge when dead entries exceed θ × live entries; we use θ = 1). Both expiry and retraction are `retract(id)` calls.

### Structure A (two heaps, lazy deletion, purge)
- Time per operation: about c1·log2(w) for heap push, sift and rebalancing, plus an amortised purge term.
- Purge term: a purge costs about (1+θ)·w steps, and at least θ·w retractions are needed to create that many dead entries since the last purge. Amortised cost per retraction is therefore (1+θ)/θ = O(1) for fixed θ. Total: O(log w) amortised.
- Memory: heap entries stay below about (1+θ)·w, so about 2·w for θ = 1, plus a pending table of at most about θ·w entries. Without a purge, dead entries could grow without bound under an adversarial pattern.
- Effect of r: more retractions mean more dead entries and more purges, so time per operation should rise slowly with r, bounded by the O(1/θ) purge term.

### Structure B (Fenwick tree over compressed values)
- insert: rank lookup plus update, O(log U). retract: id-to-value lookup, then update, O(log U). query: binary-lifting descent, O(log U).
- Time per operation should be roughly flat in w for fixed U, and grow with log2(U).
- Memory: Fenwick array and rank table are O(U), plus an id-to-value map O(w). It does not depend on r.

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